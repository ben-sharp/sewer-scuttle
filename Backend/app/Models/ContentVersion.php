<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\HasMany;

class ContentVersion extends Model
{
    use HasFactory;

    protected $fillable = [
        'version',
        'content_hash',
        'exported_at',
        'is_active',
        'notes',
    ];

    protected function casts(): array
    {
        return [
            'exported_at' => 'datetime',
            'is_active' => 'boolean',
        ];
    }

    public function definitions(): HasMany
    {
        return $this->hasMany(ContentDefinition::class);
    }

    /**
     * Get the current active version
     */
    public static function current(): ?self
    {
        return static::where('is_active', true)->first();
    }
}
