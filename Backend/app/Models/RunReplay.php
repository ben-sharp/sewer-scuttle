<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class RunReplay extends Model
{
    use HasFactory;

    protected $fillable = [
        'run_id',
        'data',
    ];

    protected $casts = [
        'data' => 'array',
    ];

    public function run(): BelongsTo
    {
        return $this->belongsTo(Run::class);
    }
}
