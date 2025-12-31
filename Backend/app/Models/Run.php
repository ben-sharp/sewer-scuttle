<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;
use Illuminate\Database\Eloquent\Builder;

class Run extends Model
{
    use HasFactory;

    protected $fillable = [
        'player_id',
        'device_id',
        'seed_id',
        'seed',
        'max_coins',
        'max_obstacles',
        'max_track_pieces',
        'track_pieces_spawned',
        'run_hash',
        'is_suspicious',
        'score',
        'distance',
        'duration_seconds',
        'coins_collected',
        'obstacles_hit',
        'powerups_used',
        'run_data',
        'started_at',
        'completed_at',
    ];

    protected function casts(): array
    {
        return [
            'run_data' => 'array',
            'score' => 'integer',
            'distance' => 'integer',
            'duration_seconds' => 'integer',
            'coins_collected' => 'integer',
            'obstacles_hit' => 'integer',
            'powerups_used' => 'integer',
            'max_coins' => 'integer',
            'max_obstacles' => 'integer',
            'max_track_pieces' => 'integer',
            'track_pieces_spawned' => 'integer',
            'is_suspicious' => 'boolean',
            'started_at' => 'datetime',
            'completed_at' => 'datetime',
        ];
    }

    public function player(): BelongsTo
    {
        return $this->belongsTo(Player::class);
    }

    /**
     * Check if run is anonymous (has device_id but no player_id)
     */
    public function isAnonymous(): bool
    {
        return $this->device_id !== null && $this->player_id === null;
    }

    /**
     * Check if run is authenticated (has player_id)
     */
    public function isAuthenticated(): bool
    {
        return $this->player_id !== null;
    }

    /**
     * Scope: Get anonymous runs
     */
    public function scopeAnonymous(Builder $query): Builder
    {
        return $query->whereNotNull('device_id')->whereNull('player_id');
    }

    /**
     * Scope: Get authenticated runs
     */
    public function scopeAuthenticated(Builder $query): Builder
    {
        return $query->whereNotNull('player_id');
    }

    /**
     * Scope: Get runs for a specific device
     */
    public function scopeForDevice(Builder $query, string $deviceId): Builder
    {
        return $query->where('device_id', $deviceId);
    }

    /**
     * Scope: Get validated runs (not suspicious)
     */
    public function scopeValidated(Builder $query): Builder
    {
        return $query->where('is_suspicious', false);
    }

    /**
     * Scope: Get suspicious runs
     */
    public function scopeSuspicious(Builder $query): Builder
    {
        return $query->where('is_suspicious', true);
    }
}
