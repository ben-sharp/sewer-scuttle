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
        Schema::create('content_versions', function (Blueprint $table) {
            $table->id();
            $table->string('version')->unique(); // e.g., "1.2.3"
            $table->string('content_hash'); // Hash of all content
            $table->timestamp('exported_at');
            $table->boolean('is_active')->default(false);
            $table->text('notes')->nullable();
            $table->timestamps();
            
            $table->index('is_active');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('content_versions');
    }
};
