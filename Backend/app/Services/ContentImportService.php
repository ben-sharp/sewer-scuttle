<?php

namespace App\Services;

use App\Models\ContentDefinition;
use App\Models\ContentVersion;
use Illuminate\Support\Facades\Storage;
use Illuminate\Support\Facades\Cache;

class ContentImportService
{
    public function import(array $data): array
    {
        $versionName = $data['version'] ?? '1.0.0';
        $definitions = $data['definitions'] ?? [];
        
        // Create or update the version
        $version = ContentVersion::updateOrCreate(
            ['version' => $versionName],
            ['is_active' => true]
        );

        // Deactivate other versions
        ContentVersion::where('id', '!=', $version->id)->update(['is_active' => false]);

        $results = [
            'total' => count($definitions),
            'created' => 0,
            'updated' => 0,
            'errors' => 0,
        ];

        foreach ($definitions as $def) {
            try {
                $model = ContentDefinition::updateOrCreate(
                    [
                        'content_version_id' => $version->id,
                        'content_id' => $def['content_id']
                    ],
                    [
                        'name' => $def['name'],
                        'content_type' => $def['type'],
                        'properties' => $def['properties'],
                        'is_active' => true,
                    ]
                );

                if ($model->wasRecentlyCreated) {
                    $results['created']++;
                } else {
                    $results['updated']++;
                }
            } catch (\Exception $e) {
                \Log::error("Import error for definition: " . json_encode($def) . " - Error: " . $e->getMessage());
                $results['errors']++;
            }
        }

        Cache::put('content_version', $versionName);

        return $results;
    }

    public function importFromFile(string $path): array
    {
        $content = null;

        // Try absolute path first
        if (file_exists($path)) {
            $content = file_get_contents($path);
        } 
        // Try storage path
        elseif (file_exists(storage_path($path))) {
            $content = file_get_contents(storage_path($path));
        }
        // Try local storage disk
        elseif (Storage::disk('local')->exists($path)) {
            $content = Storage::disk('local')->get($path);
        }
        else {
            throw new \Exception("File not found: {$path} (checked absolute, storage_path, and local disk)");
        }

        $data = json_decode($content, true);

        if (!$data) {
            throw new \Exception("Invalid JSON in file: {$path}");
        }

        return $this->import($data);
    }
}
