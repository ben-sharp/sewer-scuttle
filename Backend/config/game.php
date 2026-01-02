<?php

return [
    'content_version' => env('GAME_CONTENT_VERSION', '1.0.0'),
    'difficulty' => [
        'tiers' => 3,
        'tracks_per_tier' => 3,
    ],
    'scoring' => [
        'points_per_meter' => 10,
        'last_legs_multiplier' => 2.0,
    ],
    'tracks' => [
        'tier_lengths' => [
            1 => ['min' => 500, 'max' => 600],
            2 => ['min' => 600, 'max' => 800],
            3 => ['min' => 1000, 'max' => 1500],
        ],
        'shop_count_range' => [1, 2],
        'shop_position_range' => [0.5, 0.7], // 50-70% through track
        'shop_item_count' => [3, 5],
        'boss_reward_count' => [3, 5],
        'reroll_costs' => [50, 100, 150, 200], // Increasing costs
    ],
];
