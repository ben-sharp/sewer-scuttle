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
            $table->renameColumn('type', 'content_type');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::table('content_definitions', function (Blueprint $table) {
            $table->renameColumn('content_type', 'type');
        });
    }
};
