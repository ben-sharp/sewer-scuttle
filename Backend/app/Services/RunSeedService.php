<?php

namespace App\Services;

use App\Models\ContentDefinition;
use Illuminate\Support\Facades\Cache;
use Illuminate\Support\Str;

class RunSeedService
{
    public function generate(string $deviceId = null, int $maxDistance = null, string $playerClass = 'Vanilla'): array
    {
        $seed = mt_rand(1, 2147483647);
        $seedId = Str::uuid()->toString();
        $version = $this->getContentVersion();

        // Valid classes: Vanilla, Rogue, Enforcer, Joker, Scout, Collector

        // Pre-generate everything for the run seed
        $data = [
            'seed_id' => $seedId,
            'seed' => $seed,
            'device_id' => $deviceId,
            'max_distance' => $maxDistance,
            'player_class' => $playerClass,
            'created_at' => now()->toIso8601String(),
            'content_version' => $version,
            'tiers' => [],
            'shop_items' => [],
            'boss_rewards' => [],
        ];

        mt_srand($seed);

        for ($tier = 1; $tier <= 3; $tier++) {
            $data['tiers'][$tier] = $this->generateTierTracks($tier, $seed + $tier);
            
            // Pre-generate boss rewards for this tier
            $data['boss_rewards'][$tier] = $this->generateBossRewards($tier, $seed + $tier + 100, $playerClass);
            
            // Pre-generate shop items for each track in this tier
            foreach ($data['tiers'][$tier] as $trackIndex => $track) {
                for ($shopIndex = 0; $shopIndex < $track['shop_count']; $shopIndex++) {
                    $shopKey = "{$tier}_{$trackIndex}_{$shopIndex}";
                    $data['shop_items'][$shopKey] = $this->generateShopItems($tier, $trackIndex, $shopIndex, $seed + $tier + $trackIndex + $shopIndex, $playerClass);
                }
            }
        }

        // Cache the seed data for validation later
        Cache::put("run_seed_{$seedId}", $data, now()->addHours(24));

        // Return Tier 1 data for starting the run
        return [
            'seed_id' => $seedId,
            'seed' => $seed,
            'tier' => 1,
            'tracks' => $data['tiers'][1],
            'content_version' => $version,
        ];
    }

    public function getTierTracks(string $seedId, int $tier): ?array
    {
        $runData = Cache::get("run_seed_{$seedId}");
        if (!$runData || !isset($runData['tiers'][$tier])) return null;

        return [
            'seed_id' => $seedId,
            'seed' => $runData['seed'],
            'tier' => $tier,
            'tracks' => $runData['tiers'][$tier],
            'content_version' => $runData['content_version'],
        ];
    }

    public function selectTrack(string $seedId, int $tier, int $trackIndex): ?array
    {
        $runData = Cache::get("run_seed_{$seedId}");
        if (!$runData || !isset($runData['tiers'][$tier][$trackIndex])) return null;

        $track = $runData['tiers'][$tier][$trackIndex];
        
        // Generate the actual piece sequence for this track
        // We do this on-demand to keep the cached seed smaller, but it's still deterministic
        $sequence = $this->generatePieceSequence($track, $runData['seed'] + $tier + $trackIndex);

        return [
            'piece_ids' => $sequence['piece_ids'],
            'shop_positions' => $sequence['shop_positions'],
            'boss_id' => $track['boss_id'],
            'length' => $track['length'],
            'shop_count' => $track['shop_count'],
        ];
    }

    protected function generateTierTracks(int $tier, int $seed): array
    {
        mt_srand($seed);
        $tracks = [];
        $config = config('game.tracks');
        $lengths = $config['tier_lengths'][$tier] ?? ['min' => 500, 'max' => 600];

        // Get available bosses for this tier
        $bosses = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->whereJsonContains('properties->piece_type', 'Boss')
            ->get();
        
        $bossId = $bosses->count() > 0 ? $bosses->random()->content_id : 'Boss_Generic';

        for ($i = 0; $i < 3; $i++) {
            $targetLength = mt_rand($lengths['min'], $lengths['max']);
            $shopCount = mt_rand($config['shop_count_range'][0], $config['shop_count_range'][1]);
            
            $tracks[] = [
                'id' => $i,
                'length' => $targetLength,
                'shop_count' => $shopCount,
                'boss_id' => $bossId,
            ];
        }

        return $tracks;
    }

    protected function generatePieceSequence(array $track, int $seed): array
    {
        mt_srand($seed);
        $pieceIds = [];
        $shopPositions = [];
        $currentLength = 0;

        // Add Start piece at the beginning
        $startPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->whereJsonContains('properties->piece_type', 'Start')
            ->get();
        
        if ($startPieces->count() > 0) {
            $piece = $startPieces->random();
            $pieceIds[] = $piece->content_id;
            $currentLength += ($piece->properties['length'] ?? 800) / 800.0;
        }

        // Get normal pieces
        $normalPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->whereJsonContains('properties->piece_type', 'Normal')
            ->get();

        if ($normalPieces->isEmpty()) {
            return ['piece_ids' => [], 'shop_positions' => []];
        }

        // Fill with normal pieces until target length reached
        while ($currentLength < $track['length']) {
            $piece = $normalPieces->random();
            $pieceIds[] = $piece->content_id;
            $currentLength += ($piece->properties['length'] ?? 800) / 800.0;
        }

        // Place shops at 50-70% through
        $config = config('game.tracks');
        $range = $config['shop_position_range'] ?? [0.5, 0.7];
        
        $shopPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->whereJsonContains('properties->piece_type', 'Shop')
            ->get();

        if ($shopPieces->count() > 0) {
            for ($s = 0; $s < $track['shop_count']; $s++) {
                $progress = $range[0] + (($range[1] - $range[0]) * ($s / max(1, $track['shop_count'] - 1)));
                $pos = (int)(count($pieceIds) * $progress);
                
                // Ensure unique positions
                while (in_array($pos, $shopPositions)) $pos++;
                
                if ($pos < count($pieceIds)) {
                    $shopPositions[] = $pos;
                    $pieceIds[$pos] = $shopPieces->random()->content_id;
                }
            }
        }

        // Add boss piece at the end
        $pieceIds[] = $track['boss_id'];

        return [
            'piece_ids' => $pieceIds,
            'shop_positions' => $shopPositions,
        ];
    }

    public function generateShopItems(int $tier, int $trackIndex, int $shopIndex, int $seed, string $playerClass = 'Vanilla'): array
    {
        mt_srand($seed);
        $config = config('game.tracks');
        $itemCount = mt_rand($config['shop_item_count'][0], $config['shop_item_count'][1]);

        // Filter powerups by tier availability and player class
        $tierName = "T{$tier}";
        $query = ContentDefinition::where('content_type', 'powerup')
            ->where('is_active', true)
            ->whereJsonContains('properties->difficulty_availability', $tierName);
        
        $powerups = $query->get()->filter(function($pu) use ($playerClass) {
            $allowed = $pu->properties['allowed_classes'] ?? [];
            return count($allowed) === 0 || in_array($playerClass, $allowed);
        });

        if ($powerups->isEmpty()) {
            $powerups = ContentDefinition::where('content_type', 'powerup')->where('is_active', true)->get();
        }

        $items = [];
        $selected = $powerups->count() > $itemCount ? $powerups->random($itemCount) : $powerups;

        foreach ($selected as $pu) {
            $baseCost = $pu->properties['base_cost'] ?? 100;
            $multiplier = $pu->properties['cost_multiplier_per_tier'] ?? 0.5;
            $cost = (int)($baseCost * (1 + ($tier - 1) * $multiplier));

            $items[] = [
                'id' => $pu->content_id,
                'name' => $pu->name,
                'cost' => $cost,
                'properties' => $pu->properties,
            ];
        }

        return ['items' => $items];
    }

    public function generateBossRewards(int $tier, int $seed, string $playerClass = 'Vanilla'): array
    {
        mt_srand($seed);
        $config = config('game.tracks');
        $rewardCount = mt_rand($config['boss_reward_count'][0], $config['boss_reward_count'][1]);

        $tierName = "T{$tier}";
        $query = ContentDefinition::where('content_type', 'powerup')
            ->where('is_active', true)
            ->whereJsonContains('properties->difficulty_availability', $tierName);
        
        $powerups = $query->get()->filter(function($pu) use ($playerClass) {
            $allowed = $pu->properties['allowed_classes'] ?? [];
            return count($allowed) === 0 || in_array($playerClass, $allowed);
        });

        if ($powerups->isEmpty()) {
            $powerups = ContentDefinition::where('content_type', 'powerup')->where('is_active', true)->get();
        }

        $rewards = [];
        $selected = $powerups->count() > $rewardCount ? $powerups->random($rewardCount) : $powerups;

        foreach ($selected as $pu) {
            $rewards[] = [
                'id' => $pu->content_id,
                'name' => $pu->name,
                'properties' => $pu->properties,
            ];
        }

        return ['rewards' => $rewards];
    }

    public function getShopItems(string $seedId, int $tier, int $trackIndex, int $shopIndex): array
    {
        $runData = Cache::get("run_seed_{$seedId}");
        $shopKey = "{$tier}_{$trackIndex}_{$shopIndex}";
        
        if ($runData && isset($runData['shop_items'][$shopKey])) {
            return $runData['shop_items'][$shopKey];
        }

        // Fallback to on-demand generation if not in cache
        return $this->generateShopItems($tier, $trackIndex, $shopIndex, ($runData['seed'] ?? mt_rand()) + $tier + $trackIndex + $shopIndex, $runData['player_class'] ?? 'Vanilla');
    }

    public function getBossRewards(string $seedId, int $tier): array
    {
        $runData = Cache::get("run_seed_{$seedId}");
        
        if ($runData && isset($runData['boss_rewards'][$tier])) {
            return $runData['boss_rewards'][$tier];
        }

        // Fallback to on-demand generation
        return $this->generateBossRewards($tier, ($runData['seed'] ?? mt_rand()) + $tier + 999, $runData['player_class'] ?? 'Vanilla');
    }

    protected function getContentVersion(): string
    {
        return Cache::get('content_version', config('game.content_version', '1.0.0'));
    }
}
