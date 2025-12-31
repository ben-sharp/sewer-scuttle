<?php

namespace App\Services;

use App\Services\RunSeedService;
use Illuminate\Support\Facades\Hash;

class RunValidationService
{
    public function __construct(
        protected RunSeedService $seedService
    ) {
    }

    /**
     * Validate run data against seed
     *
     * @param array $runData
     * @param string $seedId
     * @return array Validation result with 'valid', 'suspicious', 'errors'
     */
    public function validate(array $runData, string $seedId): array
    {
        $seedData = $this->seedService->getSeedData($seedId);

        if (!$seedData) {
            return [
                'valid' => false,
                'suspicious' => false,
                'errors' => ['Seed expired or invalid'],
            ];
        }

        $errors = [];
        $suspicious = false;

        // TEMPORARILY DISABLED: All validation rules disabled for development
        // Validation structure kept intact for easy re-enabling later

        // Validate coins - DISABLED
        // $coinsCollected = $runData['coins_collected'] ?? 0;
        // if ($coinsCollected > $seedData['max_coins']) {
        //     $errors[] = "Coins collected ({$coinsCollected}) exceeds maximum possible ({$seedData['max_coins']})";
        // }

        // Validate obstacles - DISABLED
        // $obstaclesHit = $runData['obstacles_hit'] ?? 0;
        // if ($obstaclesHit > $seedData['max_obstacles']) {
        //     $errors[] = "Obstacles hit ({$obstaclesHit}) exceeds maximum possible ({$seedData['max_obstacles']})";
        // }

        // Validate distance - DISABLED
        // $distance = $runData['distance'] ?? 0;
        // if ($distance > $seedData['max_distance']) {
        //     $errors[] = "Distance ({$distance}) exceeds maximum ({$seedData['max_distance']})";
        // }

        // Validate track pieces - DISABLED
        // if (isset($runData['track_pieces_spawned'])) {
        //     $trackPiecesSpawned = $runData['track_pieces_spawned'];
        //     if ($trackPiecesSpawned > $seedData['max_track_pieces']) {
        //         $errors[] = "Track pieces spawned ({$trackPiecesSpawned}) exceeds maximum ({$seedData['max_track_pieces']})";
        //     }

        //     // Check if track pieces correlate with distance
        //     $avgLength = $seedData['max_distance'] / max($seedData['max_track_pieces'], 1);
        //     $expectedPieces = (int) ceil($distance / $avgLength);
        //     if ($trackPiecesSpawned > $expectedPieces * 1.5) {
        //         $suspicious = true;
        //     }
        // }

        // Validate score relationships - DISABLED
        // $score = $runData['score'] ?? 0;
        // $maxScore = $this->calculateMaxScore($runData, $seedData);
        // if ($score > $maxScore) {
        //     $suspicious = true;
        // }

        // Validate timestamps - DISABLED
        // $timestampErrors = $this->validateTimestamps($runData);
        // $errors = array_merge($errors, $timestampErrors);

        // Check for duplicates - DISABLED
        $runHash = $this->generateRunHash($runData, $seedId);
        // if ($this->checkDuplicate($runHash)) {
        //     $errors[] = 'Duplicate run detected';
        // }

        return [
            'valid' => empty($errors),
            'suspicious' => $suspicious,
            'errors' => $errors,
            'run_hash' => $runHash,
        ];
    }

    /**
     * Check if run hash already exists (duplicate)
     *
     * @param string $runHash
     * @return bool
     */
    public function checkDuplicate(string $runHash): bool
    {
        return \App\Models\Run::where('run_hash', $runHash)->exists();
    }

    /**
     * Validate timestamps
     *
     * @param array $runData
     * @return array Errors
     */
    protected function validateTimestamps(array $runData): array
    {
        $errors = [];

        if (isset($runData['started_at'])) {
            $startedAt = \Carbon\Carbon::parse($runData['started_at']);

            // TEMPORARILY DISABLED: All timestamp validation disabled for development
            // Check if started_at is in the future - DISABLED for dev servers with timezone issues
            // if ($startedAt->isFuture()) {
            //     $errors[] = 'Started timestamp is in the future';
            // }

            // Check if started_at is too old (more than 24 hours) - DISABLED
            // if ($startedAt->lt(now()->subDay())) {
            //     $errors[] = 'Started timestamp is too old (more than 24 hours)';
            // }
        }

        return $errors;
    }

    /**
     * Calculate max score based on distance and coins
     *
     * @param array $runData
     * @param array $seedData
     * @return int
     */
    protected function calculateMaxScore(array $runData, array $seedData): int
    {
        $distance = $runData['distance'] ?? 0;
        $coinsCollected = $runData['coins_collected'] ?? 0;

        // Simple score calculation: distance * multiplier + coins
        // This should match client-side calculation
        $scorePerDistance = 10; // Base score per distance unit
        $scorePerCoin = 1; // Score per coin

        $maxScore = ($distance * $scorePerDistance) + ($coinsCollected * $scorePerCoin);

        // Add tolerance (20%)
        return (int) ($maxScore * 1.2);
    }

    /**
     * Generate hash for duplicate detection
     *
     * @param array $runData
     * @param string $seedId
     * @return string
     */
    protected function generateRunHash(array $runData, string $seedId): string
    {
        $hashData = [
            'seed_id' => $seedId,
            'score' => $runData['score'] ?? 0,
            'distance' => $runData['distance'] ?? 0,
            'duration_seconds' => $runData['duration_seconds'] ?? 0,
            'coins_collected' => $runData['coins_collected'] ?? 0,
            'started_at' => $runData['started_at'] ?? '',
        ];

        return hash('sha256', json_encode($hashData));
    }
}

