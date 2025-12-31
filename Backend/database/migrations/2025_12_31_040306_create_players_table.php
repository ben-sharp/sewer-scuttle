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
        Schema::create('players', function (Blueprint $table) {
            $table->id();
            $table->foreignId('user_id')->constrained()->onDelete('cascade');
            $table->string('username')->unique();
            $table->string('display_name');
            $table->string('player_class')->nullable();
            $table->json('customizations')->nullable();
            $table->bigInteger('total_coins')->default(0);
            $table->bigInteger('total_distance')->default(0);
            $table->integer('total_runs')->default(0);
            $table->bigInteger('best_score')->default(0);
            $table->timestamps();
            
            $table->index('username');
            $table->index('player_class');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('players');
    }
};
