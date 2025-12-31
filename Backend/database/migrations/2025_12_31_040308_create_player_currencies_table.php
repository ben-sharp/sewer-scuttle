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
        Schema::create('player_currencies', function (Blueprint $table) {
            $table->id();
            $table->foreignId('player_id')->constrained()->onDelete('cascade');
            $table->string('currency_type'); // e.g., 'coins', 'gems', 'tokens'
            $table->bigInteger('amount')->default(0);
            $table->timestamps();
            
            $table->unique(['player_id', 'currency_type']);
            $table->index('currency_type');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('player_currencies');
    }
};
