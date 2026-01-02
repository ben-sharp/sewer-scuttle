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
        Schema::table('runs', function (Blueprint $table) {
            $table->integer('current_tier')->default(1);
            $table->integer('track_currency')->default(0);
            $table->boolean('is_complete')->default(false);
            $table->boolean('is_endless')->default(false);
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::table('runs', function (Blueprint $table) {
            $table->dropColumn(['current_tier', 'track_currency', 'is_complete', 'is_endless']);
        });
    }
};
