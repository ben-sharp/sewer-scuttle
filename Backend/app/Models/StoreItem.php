<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\HasMany;

class StoreItem extends Model
{
    use HasFactory;

    protected $fillable = [
        'name',
        'description',
        'item_type',
        'metadata',
        'currency_type',
        'price',
        'is_active',
    ];

    protected function casts(): array
    {
        return [
            'metadata' => 'array',
            'price' => 'integer',
            'is_active' => 'boolean',
        ];
    }

    public function purchases(): HasMany
    {
        return $this->hasMany(Purchase::class);
    }
}
