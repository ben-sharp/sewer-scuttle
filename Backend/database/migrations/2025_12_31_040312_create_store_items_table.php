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
        Schema::create('store_items', function (Blueprint $table) {
            $table->id();
            $table->string('name');
            $table->text('description')->nullable();
            $table->string('item_type'); // cosmetic, class_unlock, powerup, etc.
            $table->json('metadata')->nullable(); // Additional item data
            $table->string('currency_type'); // coins, gems, etc.
            $table->bigInteger('price');
            $table->boolean('is_active')->default(true);
            $table->timestamps();
            
            $table->index('item_type');
            $table->index('is_active');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('store_items');
    }
};
