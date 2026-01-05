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
        Schema::table('players', function (Blueprint $table) {
            $table->foreignId('user_id')->nullable()->change();
            $table->string('username')->nullable()->change();
            $table->string('display_name')->nullable()->change();
            $table->string('device_id')->nullable()->index()->after('user_id');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::table('players', function (Blueprint $table) {
            $table->foreignId('user_id')->nullable(false)->change();
            $table->string('username')->nullable(false)->change();
            $table->string('display_name')->nullable(false)->change();
            $table->dropColumn('device_id');
        });
    }
};
