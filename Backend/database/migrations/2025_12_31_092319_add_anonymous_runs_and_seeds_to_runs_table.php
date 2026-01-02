<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    public function up(): void
    {
        Schema::table('runs', function (Blueprint $table) {
            $table->foreignId('user_id')->nullable()->constrained()->onDelete('cascade');
            $table->string('device_id')->nullable()->index();
            $table->string('seed_id')->nullable()->index();
            $table->integer('track_pieces_spawned')->default(0);
            $table->json('track_sequence')->nullable();
            $table->boolean('is_suspicious')->default(false);
            $table->timestamp('started_at')->nullable();
        });
    }

    public function down(): void
    {
        Schema::table('runs', function (Blueprint $table) {
            $table->dropForeign(['user_id']);
            $table->dropColumn(['user_id', 'device_id', 'seed_id', 'track_pieces_spawned', 'track_sequence', 'is_suspicious', 'started_at']);
        });
    }
};
