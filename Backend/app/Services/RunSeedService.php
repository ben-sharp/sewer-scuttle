<?php

namespace App\Services;

use App\Models\ContentDefinition;
use App\Models\ContentVersion;
use Illuminate\Support\Facades\Cache;
use Illuminate\Support\Str;

class RunSeedService
{
    /**
     * Generate a new seed for a run
     *
     * @param int|null $playerId
     * @param string|null $deviceId
     * @param int|null $maxDistance
     * @return array Seed data
     */
    public function generateSeed(?int $playerId = null, ?string $deviceId = null, ?int $maxDistance = null): array
    {
        // Get current content version
        $contentVersion = ContentVersion::current();
        if (!$contentVersion) {
            throw new \RuntimeException('No active content version found');
        }

        // Generate random seed
        $seed = mt_rand(1, 2147483647);
        $seedId = Str::uuid()->toString();

        // Use config max_distance if not provided
        $maxDistance = $maxDistance ?? config('game.runs.max_distance', 10000);

        // Calculate max values using content
        $maxCoins = $this->calculateMaxCoins($seed, $contentVersion->version, $maxDistance);
        $maxObstacles = $this->calculateMaxObstacles($seed, $contentVersion->version, $maxDistance);
        $maxTrackPieces = $this->calculateMaxTrackPieces($seed, $contentVersion->version, $maxDistance);

        $seedData = [
            'seed_id' => $seedId,
            'seed' => $seed,
            'content_version' => $contentVersion->version,
            'max_coins' => $maxCoins,
            'max_obstacles' => $maxObstacles,
            'max_powerups' => 0, // TODO: Calculate from content
            'max_track_pieces' => $maxTrackPieces,
            'max_distance' => $maxDistance,
            'player_id' => $playerId,
            'device_id' => $deviceId,
            'generated_at' => now()->toIso8601String(),
            'expires_at' => now()->addHour()->toIso8601String(),
        ];

        // Store in cache with 1 hour TTL
        Cache::put("run_seed:{$seedId}", $seedData, now()->addHour());

        return $seedData;
    }

    /**
     * Get seed data by seed_id
     *
     * @param string $seedId
     * @return array|null
     */
    public function getSeedData(string $seedId): ?array
    {
        return Cache::get("run_seed:{$seedId}");
    }

    /**
     * Calculate max coins for a seed
     *
     * @param int $seed
     * @param string $contentVersion
     * @param int $maxDistance
     * @return int
     */
    protected function calculateMaxCoins(int $seed, string $contentVersion, int $maxDistance): int
    {
        $contentVersionModel = ContentVersion::where('version', $contentVersion)->first();
        if (!$contentVersionModel) {
            return 0;
        }

        // Get track piece definitions
        $trackPieces = ContentDefinition::where('content_version_id', $contentVersionModel->id)
            ->ofType('track_piece')
            ->active()
            ->get();

        if ($trackPieces->isEmpty()) {
            return 0;
        }

        // Simulate track generation to count max coins
        $totalCoins = 0;
        $currentDistance = 0;
        mt_srand($seed); // Seed the random number generator

        while ($currentDistance < $maxDistance) {
            // Select random track piece (weighted)
            $trackPiece = $this->selectTrackPiece($trackPieces);
            if (!$trackPiece) {
                break;
            }

            $length = $trackPiece->properties['length'] ?? 1000;
            $spawnConfigsRaw = $trackPiece->properties['spawn_configs'] ?? [];
            
            // Handle spawn_configs - might be JSON string or array
            $spawnConfigs = is_string($spawnConfigsRaw) 
                ? json_decode($spawnConfigsRaw, true) ?? [] 
                : $spawnConfigsRaw;

            // Count coins in this track piece
            foreach ($spawnConfigs as $config) {
                if (is_array($config) && ($config['spawn_type'] ?? '') === 'coin' && mt_rand(0, 100) / 100 <= ($config['spawn_probability'] ?? 0)) {
                    $coinValue = $config['properties']['value'] ?? 1;
                    $totalCoins += $coinValue;
                }
            }

            $currentDistance += $length;
        }

        mt_srand(); // Reset random seed

        return (int) $totalCoins;
    }

    /**
     * Calculate max obstacles for a seed
     *
     * @param int $seed
     * @param string $contentVersion
     * @param int $maxDistance
     * @return int
     */
    protected function calculateMaxObstacles(int $seed, string $contentVersion, int $maxDistance): int
    {
        $contentVersionModel = ContentVersion::where('version', $contentVersion)->first();
        if (!$contentVersionModel) {
            return 0;
        }

        $trackPieces = ContentDefinition::where('content_version_id', $contentVersionModel->id)
            ->ofType('track_piece')
            ->active()
            ->get();

        if ($trackPieces->isEmpty()) {
            return 0;
        }

        $totalObstacles = 0;
        $currentDistance = 0;
        mt_srand($seed);

        while ($currentDistance < $maxDistance) {
            $trackPiece = $this->selectTrackPiece($trackPieces);
            if (!$trackPiece) {
                break;
            }

            $length = $trackPiece->properties['length'] ?? 1000;
            $spawnConfigsRaw = $trackPiece->properties['spawn_configs'] ?? [];
            
            // Handle spawn_configs - might be JSON string or array
            $spawnConfigs = is_string($spawnConfigsRaw) 
                ? json_decode($spawnConfigsRaw, true) ?? [] 
                : $spawnConfigsRaw;

            foreach ($spawnConfigs as $config) {
                if (is_array($config) && ($config['spawn_type'] ?? '') === 'obstacle' && mt_rand(0, 100) / 100 <= ($config['spawn_probability'] ?? 0)) {
                    $totalObstacles++;
                }
            }

            $currentDistance += $length;
        }

        mt_srand();

        return (int) $totalObstacles;
    }

    /**
     * Calculate max track pieces for a seed
     *
     * @param int $seed
     * @param string $contentVersion
     * @param int $maxDistance
     * @return int
     */
    protected function calculateMaxTrackPieces(int $seed, string $contentVersion, int $maxDistance): int
    {
        $contentVersionModel = ContentVersion::where('version', $contentVersion)->first();
        if (!$contentVersionModel) {
            return 0;
        }

        $trackPieces = ContentDefinition::where('content_version_id', $contentVersionModel->id)
            ->ofType('track_piece')
            ->active()
            ->get();

        if ($trackPieces->isEmpty()) {
            return 0;
        }

        // Calculate average track piece length
        $totalLength = 0;
        $count = 0;
        foreach ($trackPieces as $piece) {
            $length = $piece->properties['length'] ?? 1000;
            $totalLength += $length;
            $count++;
        }
        $avgLength = $count > 0 ? $totalLength / $count : 1000;

        // Calculate max track pieces with safety multiplier
        $multiplier = config('game.runs.max_track_pieces_multiplier', 1.2);
        $maxTrackPieces = (int) ceil(($maxDistance / $avgLength) * $multiplier);

        return $maxTrackPieces;
    }

    /**
     * Select a track piece based on weight (simplified)
     *
     * @param \Illuminate\Database\Eloquent\Collection $trackPieces
     * @return ContentDefinition|null
     */
    protected function selectTrackPiece($trackPieces)
    {
        if ($trackPieces->isEmpty()) {
            return null;
        }

        // Simple weighted selection
        $totalWeight = $trackPieces->sum(function ($piece) {
            return $piece->properties['weight'] ?? 1;
        });

        if ($totalWeight <= 0) {
            return $trackPieces->first();
        }

        $random = mt_rand(0, $totalWeight - 1);
        $currentWeight = 0;

        foreach ($trackPieces as $piece) {
            $weight = $piece->properties['weight'] ?? 1;
            $currentWeight += $weight;
            if ($random < $currentWeight) {
                return $piece;
            }
        }

        return $trackPieces->first();
    }
}

