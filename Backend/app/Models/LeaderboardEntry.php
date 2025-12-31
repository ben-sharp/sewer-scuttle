<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class LeaderboardEntry extends Model
{
    use HasFactory;

    protected $fillable = [
        'player_id',
        'score',
        'player_class',
        'timeframe',
        'achieved_at',
    ];

    protected function casts(): array
    {
        return [
            'score' => 'integer',
            'achieved_at' => 'datetime',
        ];
    }

    public function player(): BelongsTo
    {
        return $this->belongsTo(Player::class);
    }
}
