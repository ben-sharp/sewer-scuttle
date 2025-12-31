<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;
use Illuminate\Support\Facades\DB;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        // Check if columns already exist to make migration idempotent
        $hasDeviceId = Schema::hasColumn('runs', 'device_id');
        $hasIsCompleted = Schema::hasColumn('runs', 'is_completed');
        
        // Drop old index that included is_completed FIRST (before dropping column)
        if ($hasIsCompleted) {
            try {
                Schema::table('runs', function (Blueprint $table) {
                    $table->dropIndex(['player_id', 'is_completed']);
                });
            } catch (\Exception $e) {
                // Index might not exist, continue
            }
        }
        
        Schema::table('runs', function (Blueprint $table) use ($hasDeviceId, $hasIsCompleted) {
            // Make player_id nullable
            $table->foreignId('player_id')->nullable()->change();
            
            // Remove is_completed (all runs are completed) - must be done before adding new columns
            if ($hasIsCompleted) {
                $table->dropColumn('is_completed');
            }
            
            // Add device_id for anonymous runs (only if it doesn't exist)
            if (!$hasDeviceId) {
                $table->string('device_id')->nullable()->after('player_id');
            }
            
            // Add seed-related fields (only if they don't exist)
            if (!Schema::hasColumn('runs', 'seed_id')) {
                $table->string('seed_id')->nullable()->after('device_id');
            }
            if (!Schema::hasColumn('runs', 'seed')) {
                $table->string('seed')->nullable()->after('seed_id');
            }
            
            // Add max values for validation
            if (!Schema::hasColumn('runs', 'max_coins')) {
                $table->integer('max_coins')->nullable()->after('seed');
            }
            if (!Schema::hasColumn('runs', 'max_obstacles')) {
                $table->integer('max_obstacles')->nullable()->after('max_coins');
            }
            if (!Schema::hasColumn('runs', 'max_track_pieces')) {
                $table->integer('max_track_pieces')->nullable()->after('max_obstacles');
            }
            if (!Schema::hasColumn('runs', 'track_pieces_spawned')) {
                $table->integer('track_pieces_spawned')->nullable()->after('max_track_pieces');
            }
            
            // Add validation fields
            if (!Schema::hasColumn('runs', 'run_hash')) {
                $table->string('run_hash')->nullable()->after('track_pieces_spawned');
            }
            if (!Schema::hasColumn('runs', 'is_suspicious')) {
                $table->boolean('is_suspicious')->default(false)->after('run_hash');
            }
        });
        
        // Add indexes after columns are created (only if they don't exist)
        Schema::table('runs', function (Blueprint $table) {
            if (!$this->indexExists('runs', 'runs_device_id_index')) {
                $table->index(['device_id']);
            }
            if (!$this->indexExists('runs', 'runs_seed_id_index')) {
                $table->index(['seed_id']);
            }
            if (!$this->indexExists('runs', 'runs_run_hash_index')) {
                $table->index(['run_hash']);
            }
        });
        
        // Add CHECK constraint via raw SQL (only if it doesn't exist)
        try {
            DB::statement('ALTER TABLE runs ADD CONSTRAINT runs_player_or_device_check CHECK (player_id IS NOT NULL OR device_id IS NOT NULL)');
        } catch (\Exception $e) {
            // Constraint might already exist, continue
        }
    }
    
    private function indexExists(string $table, string $index): bool
    {
        $connection = Schema::getConnection();
        $database = $connection->getDatabaseName();
        
        if ($connection->getDriverName() === 'sqlite') {
            $indexes = $connection->select("SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? AND name=?", [$table, $index]);
            return count($indexes) > 0;
        }
        
        return false;
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        // Drop CHECK constraint first
        DB::statement('ALTER TABLE runs DROP CONSTRAINT IF EXISTS runs_player_or_device_check');
        
        Schema::table('runs', function (Blueprint $table) {
            // Drop new indexes
            $table->dropIndex(['device_id']);
            $table->dropIndex(['seed_id']);
            $table->dropIndex(['run_hash']);
            
            // Drop new columns
            $table->dropColumn([
                'device_id',
                'seed_id',
                'seed',
                'max_coins',
                'max_obstacles',
                'max_track_pieces',
                'track_pieces_spawned',
                'run_hash',
                'is_suspicious',
            ]);
            
            // Make player_id required again
            $table->foreignId('player_id')->nullable(false)->change();
            
            // Restore is_completed
            $table->boolean('is_completed')->default(false)->after('completed_at');
        });
        
        // Restore old index after column is restored
        Schema::table('runs', function (Blueprint $table) {
            $table->index(['player_id', 'is_completed']);
        });
    }
};
