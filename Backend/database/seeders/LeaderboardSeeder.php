<?php

namespace Database\Seeders;

use App\Models\LeaderboardEntry;
use App\Models\Player;
use App\Models\Run;
use App\Models\RunReplay;
use Illuminate\Database\Seeder;
use Illuminate\Support\Str;

class LeaderboardSeeder extends Seeder
{
    public function run(): void
    {
        $classes = ['Vanilla', 'Rogue', 'Enforcer', 'Joker'];
        
        // Create 10 dummy players
        for ($i = 0; $i < 10; $i++) {
            $player = Player::create([
                'display_name' => 'Player_' . ($i + 1),
                'username' => 'user_' . Str::random(5),
                'device_id' => Str::uuid(),
                'total_runs' => rand(1, 50),
                'best_score' => 0,
            ]);

            // Create 3 runs for each player
            foreach (range(1, 3) as $runIndex) {
                $score = rand(1000, 50000);
                $class = $classes[array_rand($classes)];
                
                if ($score > $player->best_score) {
                    $player->update(['best_score' => $score]);
                }

                $run = Run::create([
                    'player_id' => $player->id,
                    'player_class' => $class,
                    'seed_id' => 'seeded_run',
                    'track_seed' => rand(1, 999999),
                    'score' => $score,
                    'distance' => rand(100, 5000),
                    'duration_seconds' => rand(30, 600),
                    'is_complete' => true,
                    'started_at' => now()->subDays(rand(0, 10)),
                ]);

                // Create dummy replay data (just a few jump/move events)
                RunReplay::create([
                    'run_id' => $run->id,
                    'data' => [
                        ['t' => 1.0, 'e' => 2, 'p' => ['x' => 0, 'y' => 0, 'z' => 100]], // Jump at 1s
                        ['t' => 2.5, 'e' => 0, 'p' => ['x' => 0, 'y' => -100, 'z' => 100]], // MoveLeft at 2.5s
                        ['t' => 4.0, 'e' => 1, 'p' => ['x' => 0, 'y' => 0, 'z' => 100]], // MoveRight back at 4s
                    ],
                ]);

                // Add to leaderboards
                foreach (['daily', 'weekly', 'all-time'] as $timeframe) {
                    LeaderboardEntry::create([
                        'player_id' => $player->id,
                        'run_id' => $run->id,
                        'score' => $score,
                        'player_class' => $class,
                        'timeframe' => $timeframe,
                        'achieved_at' => $run->started_at,
                    ]);
                }
            }
        }
    }
}
