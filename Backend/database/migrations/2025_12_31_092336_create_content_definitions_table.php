<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('content_definitions', function (Blueprint $table) {
            $table->id();
            $table->foreignId('content_version_id')->constrained()->onDelete('cascade');
            $table->string('content_type'); // track_piece, obstacle, powerup, collectible
            $table->string('content_id'); // Unique ID from UE (data asset name/path)
            $table->string('name');
            $table->json('properties'); // Type-specific properties
            $table->boolean('is_active')->default(true);
            $table->timestamps();
            
            $table->index(['content_version_id', 'content_type']);
            $table->unique(['content_version_id', 'content_id'], 'content_version_id_unique');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('content_definitions');
    }
};
