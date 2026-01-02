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
        $sequence = $this->generatePieceSequence($track, $runData['seed'] + $tier + $trackIndex, $runData['player_class'] ?? 'Vanilla');

        return [
            'pieces' => $sequence['pieces'],
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
            ->get()
            ->filter(function($p) {
                $type = strtolower(trim($p->properties['piece_type'] ?? ''));
                return $type === 'boss';
            });
        
        \Log::info("RunSeedService: Found " . $bosses->count() . " boss pieces.");

        $bossId = 'Boss_Generic';
        if ($bosses->count() > 0) {
            $index = mt_rand(0, $bosses->count() - 1);
            $bossId = $bosses->values()->get($index)->content_id;
        }

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

    protected function generatePieceSequence(array $track, int $seed, string $playerClass = 'Vanilla'): array
    {
        mt_srand($seed);
        $pieces = [];
        $shopPositions = [];
        $currentLength = 0;

        // Add Start piece pool (strict)
        $startPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->get()
            ->filter(function($p) {
                $type = strtolower(trim($p->properties['piece_type'] ?? ''));
                return $type === 'start';
            });
        
        \Log::info("RunSeedService: Found " . $startPieces->count() . " potential start pieces.");

        if ($startPieces->count() > 0) {
            $index = mt_rand(0, $startPieces->count() - 1);
            $piece = $startPieces->values()->get($index);
            $pieces[] = $this->prescribeSpawns($piece, $playerClass);
            $currentLength += ($piece->properties['length'] ?? 1600) / 800.0;
            \Log::info("RunSeedService: Added start piece: " . $piece->content_id);
        }

        // Get normal pieces (STRICTLY EXCLUDE Start, Shop, Boss)
        $normalPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->get()
            ->filter(function($p) {
                $props = $p->properties;
                $type = strtolower(trim($props['piece_type'] ?? 'normal'));
                
                // Exclude if explicitly Start, Shop, Boss
                if ($type === 'start' || $type === 'shop' || $type === 'boss') return false;
                
                return $type === 'normal';
            });

        \Log::info("RunSeedService: Found " . $normalPieces->count() . " normal pieces pool.");

        if ($normalPieces->isEmpty()) {
            \Log::error("RunSeedService: No normal pieces found! Sequence will be empty.");
            return ['pieces' => [], 'shop_positions' => []];
        }

        while ($currentLength < $track['length']) {
            $index = mt_rand(0, $normalPieces->count() - 1);
            $piece = $normalPieces->values()->get($index);
            $pieces[] = $this->prescribeSpawns($piece, $playerClass);
            $currentLength += ($piece->properties['length'] ?? 1600) / 800.0;
        }

        // Place shops
        $shopPieces = ContentDefinition::where('content_type', 'track_piece')
            ->where('is_active', true)
            ->get()
            ->filter(function($p) {
                $type = strtolower(trim($p->properties['piece_type'] ?? ''));
                return $type === 'shop';
            });

        \Log::info("RunSeedService: Found " . $shopPieces->count() . " shop pieces pool.");

        if ($shopPieces->count() > 0) {
            $config = config('game.tracks');
            $range = $config['shop_position_range'] ?? [0.5, 0.7];
            
            // Track index for deterministic item generation
            for ($s = 0; $s < $track['shop_count']; $s++) {
                $progress = $range[0] + (($range[1] - $range[0]) * ($s / max(1, $track['shop_count'] - 1)));
                $pos = (int)(count($pieces) * $progress);
                
                // Don't overwrite the very first (Start) or very last (Normal) piece if possible
                if ($pos <= 0) $pos = 1;
                if ($pos >= count($pieces)) $pos = count($pieces) - 1;
                
                while (in_array($pos, $shopPositions)) $pos++;
                
                if ($pos < count($pieces)) {
                    $shopPositions[] = $pos;
                    $shopIndex = mt_rand(0, $shopPieces->count() - 1);
                    $piece = $shopPieces->values()->get($shopIndex);
                    \Log::info("RunSeedService: Placing shop at index {$pos}: " . $piece->content_id);
                    $pieces[$pos] = $this->prescribeSpawns($piece, $playerClass);
                }
            }
        }

        // Add Boss
        $bossPiece = ContentDefinition::where('content_id', $track['boss_id'])->first();
        if ($bossPiece) {
            \Log::info("RunSeedService: Adding boss piece: " . $bossPiece->content_id);
            $pieces[] = $this->prescribeSpawns($bossPiece, $playerClass);
        } else {
            \Log::warning("RunSeedService: Boss piece " . $track['boss_id'] . " not found!");
        }

        \Log::info("RunSeedService: Generated sequence with " . count($pieces) . " pieces.");

        return [
            'pieces' => $pieces,
            'shop_positions' => $shopPositions,
        ];
    }

    protected function prescribeSpawns(ContentDefinition $piece, string $playerClass): array
    {
        $spawnMap = [];
        $configs = $piece->properties['spawn_configs'] ?? [];

        foreach ($configs as $config) {
            $compName = $config['component_name'] ?? null;
            if (!$compName) continue;

            // Roll probability
            if (mt_rand(0, 1000) / 1000.0 > ($config['probability'] ?? 1.0)) {
                $spawnMap[$compName] = null;
                continue;
            }

            // Roll weighted selection
            $weighted = $config['weighted_definitions'] ?? [];
            $valid = [];
            $totalWeight = 0;

            foreach ($weighted as $wd) {
                $defId = $wd['id'];
                $def = ContentDefinition::where('content_id', $defId)->first();
                if (!$def) continue;

                $allowed = $def->properties['allowed_classes'] ?? [];
                if (count($allowed) === 0 || in_array($playerClass, $allowed)) {
                    $valid[] = $wd;
                    $totalWeight += $wd['weight'] ?? 1.0;
                }
            }

            if ($totalWeight > 0) {
                $roll = mt_rand(0, $totalWeight * 1000) / 1000.0;
                $current = 0;
                foreach ($valid as $wd) {
                    $current += $wd['weight'] ?? 1.0;
                    if ($roll <= $current) {
                        $spawnMap[$compName] = $wd['id'];
                        break;
                    }
                }
            } else {
                $spawnMap[$compName] = null;
            }
        }

        return [
            'id' => $piece->content_id,
            'spawns' => (object)$spawnMap // Ensure it's an object even if empty
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

        $selected = $powerups;
        if ($powerups->count() > $itemCount) {
            $selected = collect();
            $tempList = $powerups->values()->all();
            for ($i = 0; $i < $itemCount; $i++) {
                $idx = mt_rand(0, count($tempList) - 1);
                $selected->push($tempList[$idx]);
                array_splice($tempList, $idx, 1);
            }
        }

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

        $selected = $powerups;
        if ($powerups->count() > $rewardCount) {
            $selected = collect();
            $tempList = $powerups->values()->all();
            for ($i = 0; $i < $rewardCount; $i++) {
                $idx = mt_rand(0, count($tempList) - 1);
                $selected->push($tempList[$idx]);
                array_splice($tempList, $idx, 1);
            }
        }

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

