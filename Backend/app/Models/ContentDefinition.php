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
        'content_id',
        'name',
        'content_type',
        'properties',
        'is_active',
    ];

    protected $casts = [
        'properties' => 'array',
        'is_active' => 'boolean',
    ];

    public function contentVersion(): BelongsTo
    {
        return $this->belongsTo(ContentVersion::class);
    }
}
