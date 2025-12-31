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
        Schema::create('runs', function (Blueprint $table) {
            $table->id();
            $table->foreignId('player_id')->constrained()->onDelete('cascade');
            $table->bigInteger('score')->default(0);
            $table->bigInteger('distance')->default(0);
            $table->integer('duration_seconds')->default(0);
            $table->integer('coins_collected')->default(0);
            $table->integer('obstacles_hit')->default(0);
            $table->integer('powerups_used')->default(0);
            $table->json('run_data')->nullable(); // Additional run metadata
            $table->timestamp('started_at');
            $table->timestamp('completed_at')->nullable();
            $table->boolean('is_completed')->default(false);
            $table->timestamps();
            
            $table->index('player_id');
            $table->index(['player_id', 'is_completed']);
            $table->index('score');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('runs');
    }
};
