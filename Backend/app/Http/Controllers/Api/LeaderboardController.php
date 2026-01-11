<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\LeaderboardEntry;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class LeaderboardController extends Controller
{
    public function index(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'top' => 'sometimes|integer|min:1|max:100',
            'class' => 'sometimes|nullable|string',
            'timeframe' => 'sometimes|in:daily,weekly,all-time',
        ]);

        $top = $validated['top'] ?? 100;
        $class = $validated['class'] ?? null;
        $timeframe = $validated['timeframe'] ?? 'all-time';

        $query = LeaderboardEntry::with(['player', 'run.replay'])
            ->where('timeframe', $timeframe)
            ->whereNotIn('player_class', ['Scout', 'Collector']) // Exclude coming soon classes
            ->orderBy('score', 'desc');

        if ($class) {
            $query->where('player_class', $class);
        }

        // Filter by date for daily/weekly
        if ($timeframe === 'daily') {
            $query->whereDate('achieved_at', today());
        } elseif ($timeframe === 'weekly') {
            $query->whereBetween('achieved_at', [now()->startOfWeek(), now()->endOfWeek()]);
        }

        $entries = $query->limit($top)->get();

        return response()->json([
            'timeframe' => $timeframe,
            'class' => $class,
            'entries' => $entries->map(function ($entry, $index) {
                return [
                    'rank' => $index + 1,
                    'player_id' => $entry->player_id,
                    'run_id' => $entry->run_id,
                    'player_name' => $entry->player->display_name,
                    'score' => $entry->score,
                    'player_class' => $entry->player_class,
                        'achieved_at' => $entry->achieved_at,
                        'seed_id' => $entry->run?->seed_id,
                        'track_seed' => $entry->run?->track_seed ?? 0,
                        'has_replay' => $entry->run?->replay !== null,
                    ];
            }),
        ]);
    }

    public function submit(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'score' => 'required|integer|min:0',
            'player_class' => 'sometimes|nullable|string',
        ]);

        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        // Update player stats
        $player->increment('total_runs');
        $player->increment('total_coins', $validated['score']); // Assuming score includes coins
        if ($validated['score'] > $player->best_score) {
            $player->update(['best_score' => $validated['score']]);
        }

        // Create leaderboard entries for all timeframes
        $timeframes = ['daily', 'weekly', 'all-time'];
        foreach ($timeframes as $timeframe) {
            LeaderboardEntry::create([
                'player_id' => $player->id,
                'score' => $validated['score'],
                'player_class' => $validated['player_class'] ?? $player->player_class,
                'timeframe' => $timeframe,
                'achieved_at' => now(),
            ]);
        }

        return response()->json([
            'message' => 'Score submitted successfully',
            'player_stats' => [
                'total_runs' => $player->total_runs,
                'best_score' => $player->best_score,
                'total_coins' => $player->total_coins,
            ],
        ], 201);
    }

    public function playerRank(Request $request, int $id): JsonResponse
    {
        $validated = $request->validate([
            'timeframe' => 'sometimes|in:daily,weekly,all-time',
        ]);

        $timeframe = $validated['timeframe'] ?? 'all-time';

        $player = \App\Models\Player::findOrFail($id);

        $query = LeaderboardEntry::where('timeframe', $timeframe)
            ->orderBy('score', 'desc');

        if ($timeframe === 'daily') {
            $query->whereDate('achieved_at', today());
        } elseif ($timeframe === 'weekly') {
            $query->whereBetween('achieved_at', [now()->startOfWeek(), now()->endOfWeek()]);
        }

        $allEntries = $query->pluck('id')->toArray();
        $playerEntries = $query->where('player_id', $player->id)->pluck('id')->toArray();

        $rank = null;
        if (! empty($playerEntries)) {
            $rank = array_search($playerEntries[0], $allEntries) + 1;
        }

        $bestEntry = LeaderboardEntry::where('player_id', $player->id)
            ->where('timeframe', $timeframe)
            ->orderBy('score', 'desc')
            ->first();

        return response()->json([
            'player_id' => $player->id,
            'player_name' => $player->display_name,
            'timeframe' => $timeframe,
            'rank' => $rank,
            'best_score' => $bestEntry?->score ?? 0,
        ]);
    }
}
