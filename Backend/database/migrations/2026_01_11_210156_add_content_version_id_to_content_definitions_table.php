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
        Schema::table('content_definitions', function (Blueprint $table) {
            $table->foreignId('content_version_id')->nullable()->after('id')->constrained('content_versions')->onDelete('cascade');
            $table->boolean('is_active')->default(true)->after('properties');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::table('content_definitions', function (Blueprint $table) {
            $table->dropForeign(['content_version_id']);
            $table->dropColumn(['content_version_id', 'is_active']);
        });
    }
};
