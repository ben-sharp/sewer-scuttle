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
        Schema::create('leaderboard_entries', function (Blueprint $table) {
            $table->id();
            $table->foreignId('player_id')->constrained()->onDelete('cascade');
            $table->bigInteger('score');
            $table->string('player_class')->nullable();
            $table->string('timeframe')->default('all-time'); // daily, weekly, all-time
            $table->timestamp('achieved_at');
            $table->timestamps();
            
            $table->index(['timeframe', 'player_class', 'score']);
            $table->index(['player_id', 'timeframe']);
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('leaderboard_entries');
    }
};
