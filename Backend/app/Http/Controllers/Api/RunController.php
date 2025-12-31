<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Run;
use App\Services\RunSeedService;
use App\Services\RunValidationService;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class RunController extends Controller
{
    public function __construct(
        protected RunSeedService $seedService,
        protected RunValidationService $validationService
    ) {
    }

    /**
     * Request seed for new run
     * POST /api/runs/start
     */
    public function start(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'max_distance' => 'sometimes|integer|min:1|max:100000',
            'device_id' => 'sometimes|string',
        ]);

        $playerId = null;
        $deviceId = $validated['device_id'] ?? null;

        // Get authenticated user if available
        if ($request->user()) {
            $player = $request->user()->player;
            if ($player) {
                $playerId = $player->id;
            }
        }

        // Require either authenticated user or device_id
        if (!$playerId && !$deviceId) {
            return response()->json(['message' => 'Authentication or device_id required'], 401);
        }

        try {
            $seedData = $this->seedService->generateSeed($playerId, $deviceId, $validated['max_distance'] ?? null);

            return response()->json([
                'seed_id' => $seedData['seed_id'],
                'seed' => $seedData['seed'],
                'content_version' => $seedData['content_version'],
                'max_coins' => $seedData['max_coins'],
                'max_obstacles' => $seedData['max_obstacles'],
                'max_track_pieces' => $seedData['max_track_pieces'],
                'max_distance' => $seedData['max_distance'],
                'expires_at' => $seedData['expires_at'],
            ], 201);
        } catch (\Exception $e) {
            return response()->json([
                'message' => 'Failed to generate seed',
                'error' => $e->getMessage(),
            ], 500);
        }
    }

    /**
     * Submit completed run (single endpoint)
     * POST /api/runs
     */
    public function store(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'seed_id' => 'required|string',
            'score' => 'required|integer|min:0',
            'distance' => 'required|integer|min:0',
            'duration_seconds' => 'required|integer|min:0',
            'coins_collected' => 'sometimes|integer|min:0',
            'obstacles_hit' => 'sometimes|integer|min:0',
            'powerups_used' => 'sometimes|integer|min:0',
            'track_pieces_spawned' => 'sometimes|integer|min:0',
            'run_data' => 'sometimes|array',
            'started_at' => 'required|date',
            'device_id' => 'sometimes|string',
        ]);

        $playerId = null;
        $deviceId = $validated['device_id'] ?? null;

        // Get authenticated user if available
        if ($request->user()) {
            $player = $request->user()->player;
            if ($player) {
                $playerId = $player->id;
            }
        }

        // Require either authenticated user or device_id
        if (!$playerId && !$deviceId) {
            return response()->json(['message' => 'Authentication or device_id required'], 401);
        }

        // Get seed data
        $seedData = $this->seedService->getSeedData($validated['seed_id']);
        if (!$seedData) {
            return response()->json(['message' => 'Seed expired or invalid'], 400);
        }

        // Validate run data
        $validationResult = $this->validationService->validate($validated, $validated['seed_id']);

        if (!$validationResult['valid']) {
            return response()->json([
                'message' => 'Run validation failed',
                'errors' => $validationResult['errors'],
            ], 422);
        }

        // Create run record
        $run = Run::create([
            'player_id' => $playerId,
            'device_id' => $deviceId,
            'seed_id' => $validated['seed_id'],
            'seed' => $seedData['seed'],
            'max_coins' => $seedData['max_coins'],
            'max_obstacles' => $seedData['max_obstacles'],
            'max_track_pieces' => $seedData['max_track_pieces'],
            'track_pieces_spawned' => $validated['track_pieces_spawned'] ?? null,
            'run_hash' => $validationResult['run_hash'],
            'is_suspicious' => $validationResult['suspicious'],
            'score' => $validated['score'],
            'distance' => $validated['distance'],
            'duration_seconds' => $validated['duration_seconds'],
            'coins_collected' => $validated['coins_collected'] ?? 0,
            'obstacles_hit' => $validated['obstacles_hit'] ?? 0,
            'powerups_used' => $validated['powerups_used'] ?? 0,
            'run_data' => $validated['run_data'] ?? [],
            'started_at' => $validated['started_at'],
            'completed_at' => now(),
        ]);

        // Update player stats (only if authenticated)
        if ($playerId) {
            $player = $request->user()->player;
            $player->increment('total_runs');
            $player->increment('total_distance', $validated['distance']);
            $player->increment('total_coins', $validated['coins_collected'] ?? 0);

            if ($validated['score'] > $player->best_score) {
                $player->update(['best_score' => $validated['score']]);
            }

            // Add coins to currency
            if (($validated['coins_collected'] ?? 0) > 0) {
                $player->addCurrency('coins', $validated['coins_collected']);
            }
        }

        return response()->json([
            'message' => 'Run completed successfully',
            'run' => $run->load('player'),
            'player_stats' => $playerId ? [
                'total_runs' => $request->user()->player->total_runs,
                'total_distance' => $request->user()->player->total_distance,
                'total_coins' => $request->user()->player->total_coins,
                'best_score' => $request->user()->player->best_score,
            ] : null,
        ], 201);
    }

    /**
     * Submit completed run (legacy endpoint - redirects to store)
     * POST /api/runs/end
     */
    public function end(Request $request): JsonResponse
    {
        return $this->store($request);
    }

    /**
     * Get run history
     * GET /api/runs/history
     */
    public function history(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'limit' => 'sometimes|integer|min:1|max:100',
        ]);

        $playerId = null;
        $deviceId = null;

        // Get authenticated user if available
        if ($request->user()) {
            $player = $request->user()->player;
            if ($player) {
                $playerId = $player->id;
            }
        } else {
            // Try to get device_id from request
            $deviceId = $request->input('device_id');
        }

        if (!$playerId && !$deviceId) {
            return response()->json(['message' => 'Authentication or device_id required'], 401);
        }

        $query = Run::orderBy('created_at', 'desc');

        if ($playerId) {
            $query->where('player_id', $playerId);
        } else {
            $query->where('device_id', $deviceId);
        }

        $runs = $query->limit($validated['limit'] ?? 50)->get();

        return response()->json($runs);
    }

    /**
     * Get specific run
     * GET /api/runs/{id}
     */
    public function show(Request $request, int $id): JsonResponse
    {
        $playerId = null;
        $deviceId = null;

        if ($request->user()) {
            $player = $request->user()->player;
            if ($player) {
                $playerId = $player->id;
            }
        } else {
            $deviceId = $request->input('device_id');
        }

        if (!$playerId && !$deviceId) {
            return response()->json(['message' => 'Authentication or device_id required'], 401);
        }

        $query = Run::where('id', $id);

        if ($playerId) {
            $query->where('player_id', $playerId);
        } else {
            $query->where('device_id', $deviceId);
        }

        $run = $query->firstOrFail();

        return response()->json($run->load('player'));
    }
}
