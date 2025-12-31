<?php

namespace Database\Seeders;

use App\Models\Player;
use App\Models\User;
use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\Hash;

class AdminUserSeeder extends Seeder
{
    use WithoutModelEvents;

    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        // Create test admin user
        $admin = User::firstOrCreate(
            ['email' => 'admin@sewerscuttle.com'],
            [
                'name' => 'Admin User',
                'password' => Hash::make('password'),
                'is_admin' => true,
            ]
        );

        // Create player profile for admin if it doesn't exist
        if (! $admin->player) {
            $player = Player::create([
                'user_id' => $admin->id,
                'username' => 'admin',
                'display_name' => 'Admin',
                'player_class' => null,
                'total_coins' => 0,
                'total_distance' => 0,
                'total_runs' => 0,
                'best_score' => 0,
            ]);

            // Initialize default currency
            $player->addCurrency('coins', 0);
        }

        $this->command->info('Admin user created:');
        $this->command->info('Email: admin@sewerscuttle.com');
        $this->command->info('Password: password');
        $this->command->info('Admin Panel: /admin');
    }
}
