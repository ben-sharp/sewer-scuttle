<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class ContentDefinition extends Model
{
    use HasFactory;

    protected $fillable = [
        'content_version_id',
        'content_type',
        'content_id',
        'name',
        'properties',
        'is_active',
    ];

    protected function casts(): array
    {
        return [
            'properties' => 'array',
            'is_active' => 'boolean',
        ];
    }

    public function contentVersion(): BelongsTo
    {
        return $this->belongsTo(ContentVersion::class);
    }

    /**
     * Scope: Get definitions by type
     */
    public function scopeOfType($query, string $type)
    {
        return $query->where('content_type', $type);
    }

    /**
     * Scope: Get active definitions
     */
    public function scopeActive($query)
    {
        return $query->where('is_active', true);
    }
}
