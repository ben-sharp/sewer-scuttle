<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('content_definitions', function (Blueprint $table) {
            $table->id();
            $table->string('content_id')->unique();
            $table->string('name');
            $table->string('type'); // track_piece, powerup, obstacle, collectible
            $table->json('properties')->nullable();
            $table->timestamps();
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('content_definitions');
    }
};
