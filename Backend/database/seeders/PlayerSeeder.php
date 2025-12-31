<?php

namespace Database\Seeders;

use App\Models\Player;
use App\Models\User;
use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\Hash;

class PlayerSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        $players = [
            [
                'name' => 'Dev New Player',
                'email' => 'dev_new_player@sewerscuttle.test',
                'username' => 'dev_new_player',
                'display_name' => 'Dev New Player',
                'password' => 'password',
                'total_coins' => 0,
                'total_distance' => 0,
                'total_runs' => 0,
                'best_score' => 0,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Rich Player',
                'email' => 'dev_rich_player@sewerscuttle.test',
                'username' => 'dev_rich_player',
                'display_name' => 'Dev Rich Player',
                'password' => 'password',
                'total_coins' => 50000,
                'total_distance' => 50000,
                'total_runs' => 100,
                'best_score' => 50000,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Pro Player',
                'email' => 'dev_pro_player@sewerscuttle.test',
                'username' => 'dev_pro_player',
                'display_name' => 'Dev Pro Player',
                'password' => 'password',
                'total_coins' => 10000,
                'total_distance' => 100000,
                'total_runs' => 500,
                'best_score' => 100000,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Casual Player',
                'email' => 'dev_casual_player@sewerscuttle.test',
                'username' => 'dev_casual_player',
                'display_name' => 'Dev Casual Player',
                'password' => 'password',
                'total_coins' => 5000,
                'total_distance' => 25000,
                'total_runs' => 50,
                'best_score' => 15000,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Admin Player',
                'email' => 'dev_admin_player@sewerscuttle.test',
                'username' => 'dev_admin_player',
                'display_name' => 'Dev Admin Player',
                'password' => 'password',
                'total_coins' => 100000,
                'total_distance' => 200000,
                'total_runs' => 1000,
                'best_score' => 200000,
                'is_admin' => true,
            ],
            [
                'name' => 'Dev Noob Player',
                'email' => 'dev_noob_player@sewerscuttle.test',
                'username' => 'dev_noob_player',
                'display_name' => 'Dev Noob Player',
                'password' => 'password',
                'total_coins' => 100,
                'total_distance' => 1000,
                'total_runs' => 5,
                'best_score' => 500,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Speed Runner',
                'email' => 'dev_speedrunner@sewerscuttle.test',
                'username' => 'dev_speedrunner',
                'display_name' => 'Dev Speed Runner',
                'password' => 'password',
                'total_coins' => 15000,
                'total_distance' => 200000,
                'total_runs' => 800,
                'best_score' => 25000,
                'is_admin' => false,
            ],
            [
                'name' => 'Dev Collector',
                'email' => 'dev_collector@sewerscuttle.test',
                'username' => 'dev_collector',
                'display_name' => 'Dev Collector',
                'password' => 'password',
                'total_coins' => 75000,
                'total_distance' => 75000,
                'total_runs' => 200,
                'best_score' => 30000,
                'is_admin' => false,
            ],
        ];

        foreach ($players as $playerData) {
            // Check if user already exists
            $user = User::where('email', $playerData['email'])->first();

            if (!$user) {
                $user = User::create([
                    'name' => $playerData['name'],
                    'email' => $playerData['email'],
                    'password' => Hash::make($playerData['password']),
                    'is_admin' => $playerData['is_admin'],
                ]);
            }

            // Check if player already exists
            if (!$user->player) {
                $player = Player::create([
                    'user_id' => $user->id,
                    'username' => $playerData['username'],
                    'display_name' => $playerData['display_name'],
                    'total_coins' => $playerData['total_coins'],
                    'total_distance' => $playerData['total_distance'],
                    'total_runs' => $playerData['total_runs'],
                    'best_score' => $playerData['best_score'],
                ]);

                // Initialize currency
                $player->addCurrency('coins', $playerData['total_coins']);
            } else {
                // Update existing player stats
                $user->player->update([
                    'total_coins' => $playerData['total_coins'],
                    'total_distance' => $playerData['total_distance'],
                    'total_runs' => $playerData['total_runs'],
                    'best_score' => $playerData['best_score'],
                ]);
            }
        }
    }
}
