<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Player;
use App\Models\Run;
use App\Services\RunSeedService;
use App\Services\RunValidationService;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;
use Illuminate\Support\Str;

class RunController extends Controller
{
    public function __construct(
        protected RunSeedService $seedService,
        protected RunValidationService $validationService
    ) {}

    public function start(Request $request): JsonResponse
    {
        $deviceId = $request->input('device_id');
        $maxDistance = $request->input('max_distance');
        $playerClass = $request->input('player_class', 'Vanilla');

        $seedData = $this->seedService->generate($deviceId, $maxDistance, $playerClass);

        return response()->json($seedData);
    }

    public function getTierTracks(string $seedId, int $tier): JsonResponse
    {
        $tierTracks = $this->seedService->getTierTracks($seedId, $tier);
        if (!$tierTracks) {
            return response()->json(['message' => 'Invalid seed_id or tier'], 404);
        }

        return response()->json($tierTracks);
    }

    public function selectTrack(Request $request, string $seedId): JsonResponse
    {
        $validated = $request->validate([
            'tier' => 'required|integer',
            'track_index' => 'required|integer',
        ]);

        $sequence = $this->seedService->selectTrack($seedId, $validated['tier'], $validated['track_index']);
        if (!$sequence) {
            return response()->json(['message' => 'Invalid selection'], 404);
        }

        return response()->json($sequence);
    }

    public function getShopItems(string $seedId, int $tier, int $trackIndex, int $shopIndex): JsonResponse
    {
        $shopData = $this->seedService->getShopItems($seedId, $tier, $trackIndex, $shopIndex);
        return response()->json($shopData);
    }

    public function getBossRewards(string $seedId, int $tier): JsonResponse
    {
        $rewards = $this->seedService->getBossRewards($seedId, $tier);
        return response()->json($rewards);
    }

    public function store(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'seed_id' => 'required|string',
            'score' => 'required|integer',
            'distance' => 'required|integer',
            'duration_seconds' => 'required|integer',
            'coins_collected' => 'integer',
            'obstacles_hit' => 'integer',
            'powerups_used' => 'integer',
            'track_pieces_spawned' => 'integer',
            'track_sequence' => 'array',
            'is_complete' => 'boolean',
            'is_endless' => 'boolean',
            'player_class' => 'string|nullable',
            'device_id' => 'string|nullable',
            'started_at' => 'string|nullable',
        ]);

        $validationResult = $this->validationService->validate($validated);
        
        $player = $request->user()?->player;

        // If no authenticated player, find or create one by device_id
        if (!$player && $validated['device_id']) {
            $player = Player::firstOrCreate(
                ['device_id' => $validated['device_id']],
                [
                    'display_name' => 'Guest Player',
                    'username' => 'guest_' . Str::random(8),
                ]
            );
        }

        $run = Run::create([
            'player_id' => $player?->id,
            'device_id' => $validated['device_id'] ?? null,
            'player_class' => $validated['player_class'] ?? 'Vanilla',
            'seed_id' => $validated['seed_id'],
            'score' => $validated['score'],
            'distance' => $validated['distance'],
            'duration_seconds' => $validated['duration_seconds'],
            'coins_collected' => $validated['coins_collected'] ?? 0,
            'obstacles_hit' => $validated['obstacles_hit'] ?? 0,
            'powerups_used' => $validated['powerups_used'] ?? 0,
            'track_pieces_spawned' => $validated['track_pieces_spawned'] ?? 0,
            'track_sequence' => $validated['track_sequence'] ?? [],
            'is_complete' => $validated['is_complete'] ?? false,
            'is_endless' => $validated['is_endless'] ?? false,
            'is_suspicious' => $validationResult['is_suspicious'] ?? false,
            'started_at' => (isset($validated['started_at']) && $validated['started_at']) ? now()->parse($validated['started_at']) : now(),
        ]);

        // Update player stats
        if ($player) {
            $player->increment('total_runs');
            $player->increment('total_coins', $run->coins_collected);
            $player->increment('total_distance', $run->distance);
            if ($run->score > $player->best_score) {
                $player->update(['best_score' => $run->score]);
            }

            // Create leaderboard entries if run is complete
            if ($run->is_complete) {
                $timeframes = ['daily', 'weekly', 'all-time'];
                foreach ($timeframes as $timeframe) {
                    \App\Models\LeaderboardEntry::create([
                        'player_id' => $player->id,
                        'score' => $run->score,
                        'player_class' => $run->player_class,
                        'timeframe' => $timeframe,
                        'achieved_at' => now(),
                    ]);
                }
            }
        }

        return response()->json([
            'message' => 'Run saved',
            'id' => $run->id,
            'is_suspicious' => $run->is_suspicious,
        ], 201);
    }
}
