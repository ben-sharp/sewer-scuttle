<?php

namespace App\Services;

use App\Models\ContentDefinition;
use App\Models\ContentVersion;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Facades\Log;

class ContentImportService
{
    /**
     * Import content from JSON data
     *
     * @param array $data Content data with version and definitions
     * @return ContentVersion
     */
    public function import(array $data): ContentVersion
    {
        $version = $data['version'] ?? null;
        $definitions = $data['definitions'] ?? [];
        $exportedAt = $data['exported_at'] ?? now();

        if (!$version) {
            throw new \InvalidArgumentException('Content version is required');
        }

        // Calculate content hash
        $contentHash = $this->calculateContentHash($definitions);

        // Check if version already exists
        $existingVersion = ContentVersion::where('version', $version)->first();

        DB::beginTransaction();

        try {
            if ($existingVersion) {
                // Check if content has changed
                if ($existingVersion->content_hash === $contentHash) {
                    Log::info("Content version {$version} already exists with same hash, skipping import");
                    DB::commit();
                    return $existingVersion;
                }

                // Content has changed, update existing version
                $existingVersion->update([
                    'content_hash' => $contentHash,
                    'exported_at' => $exportedAt,
                ]);

                // Delete old definitions
                $existingVersion->definitions()->delete();

                $contentVersion = $existingVersion;
            } else {
                // Create new content version
                $contentVersion = ContentVersion::create([
                    'version' => $version,
                    'content_hash' => $contentHash,
                    'exported_at' => $exportedAt,
                    'is_active' => false, // Will be activated after import
                ]);
            }

            // Import definitions
            foreach ($definitions as $definition) {
                ContentDefinition::create([
                    'content_version_id' => $contentVersion->id,
                    'content_type' => $definition['type'] ?? 'unknown',
                    'content_id' => $definition['id'] ?? '',
                    'name' => $definition['name'] ?? '',
                    'properties' => $definition['properties'] ?? [],
                    'is_active' => true,
                ]);
            }

            // Deactivate all other versions
            ContentVersion::where('id', '!=', $contentVersion->id)
                ->update(['is_active' => false]);

            // Activate this version
            $contentVersion->update(['is_active' => true]);

            DB::commit();

            Log::info("Content version {$version} imported successfully with " . count($definitions) . " definitions");

            return $contentVersion;
        } catch (\Exception $e) {
            DB::rollBack();
            Log::error("Failed to import content version {$version}: " . $e->getMessage());
            throw $e;
        }
    }

    /**
     * Calculate hash of content definitions
     *
     * @param array $definitions
     * @return string
     */
    protected function calculateContentHash(array $definitions): string
    {
        // Sort definitions by content_id for consistent hashing
        usort($definitions, function ($a, $b) {
            $idA = $a['id'] ?? '';
            $idB = $b['id'] ?? '';
            return strcmp($idA, $idB);
        });

        // Create hash from definitions
        $contentString = json_encode($definitions, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
        return hash('sha256', $contentString);
    }

    /**
     * Import from file
     *
     * @param string $filePath
     * @return ContentVersion
     */
    public function importFromFile(string $filePath): ContentVersion
    {
        if (!file_exists($filePath)) {
            throw new \InvalidArgumentException("File not found: {$filePath}");
        }

        $content = file_get_contents($filePath);
        $data = json_decode($content, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new \InvalidArgumentException("Invalid JSON: " . json_last_error_msg());
        }

        return $this->import($data);
    }
}

