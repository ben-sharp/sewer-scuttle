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
        'player_class',
        'seed_id',
        'track_seed',
        'score',
        'distance',
        'duration_seconds',
        'coins_collected',
        'obstacles_hit',
        'powerups_used',
        'track_pieces_spawned',
        'track_sequence',
        'current_tier',
        'track_currency',
        'is_complete',
        'is_endless',
        'is_suspicious',
        'started_at',
    ];

    protected $casts = [
        'track_sequence' => 'array',
        'started_at' => 'datetime',
        'is_complete' => 'boolean',
        'is_endless' => 'boolean',
        'is_suspicious' => 'boolean',
        'track_seed' => 'integer',
        'score' => 'integer',
        'distance' => 'integer',
        'duration_seconds' => 'integer',
        'coins_collected' => 'integer',
        'obstacles_hit' => 'integer',
        'powerups_used' => 'integer',
        'track_pieces_spawned' => 'integer',
        'current_tier' => 'integer',
        'track_currency' => 'integer',
    ];

    public function player(): BelongsTo
    {
        return $this->belongsTo(Player::class);
    }

    public function replay(): \Illuminate\Database\Eloquent\Relations\HasOne
    {
        return $this->hasOne(RunReplay::class);
    }

    public function scopeAuthenticated(Builder $query): Builder
    {
        return $query->whereNotNull('player_id');
    }

    public function scopeAnonymous(Builder $query): Builder
    {
        return $query->whereNull('player_id');
    }
}
