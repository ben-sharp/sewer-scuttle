<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;
use Illuminate\Database\Eloquent\Relations\HasMany;

class Player extends Model
{
    use HasFactory;

    protected $fillable = [
        'user_id',
        'username',
        'display_name',
        'player_class',
        'customizations',
        'total_coins',
        'total_distance',
        'total_runs',
        'best_score',
    ];

    protected function casts(): array
    {
        return [
            'customizations' => 'array',
            'total_coins' => 'integer',
            'total_distance' => 'integer',
            'total_runs' => 'integer',
            'best_score' => 'integer',
        ];
    }

    public function user(): BelongsTo
    {
        return $this->belongsTo(User::class);
    }

    public function currencies(): HasMany
    {
        return $this->hasMany(PlayerCurrency::class);
    }

    public function leaderboardEntries(): HasMany
    {
        return $this->hasMany(LeaderboardEntry::class);
    }

    public function purchases(): HasMany
    {
        return $this->hasMany(Purchase::class);
    }

    public function runs(): HasMany
    {
        return $this->hasMany(Run::class);
    }

    public function getCurrency(string $type): ?PlayerCurrency
    {
        return $this->currencies()->where('currency_type', $type)->first();
    }

    public function addCurrency(string $type, int $amount): void
    {
        $currency = $this->getCurrency($type);
        if ($currency) {
            $currency->increment('amount', $amount);
        } else {
            $this->currencies()->create([
                'currency_type' => $type,
                'amount' => $amount,
            ]);
        }
    }
}
