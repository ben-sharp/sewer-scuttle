<?php

namespace App\Services;

use App\Models\Run;
use Illuminate\Support\Facades\Cache;

class RunValidationService
{
    public function validate(array $data): array
    {
        $errors = [];
        $seedId = $data['seed_id'] ?? null;
        
        if (!$seedId) {
            $errors[] = "Missing seed_id";
            return ['is_valid' => false, 'errors' => $errors, 'is_suspicious' => true];
        }

        $seedData = Cache::get("run_seed_{$seedId}");
        
        if (!$seedData) {
            // Mark as suspicious if seed not found (expired or spoofed)
            return ['is_valid' => true, 'errors' => ["Seed not found in cache"], 'is_suspicious' => true];
        }

        // Basic score validation (rough estimate)
        $expectedMaxScore = $data['distance'] * config('game.scoring.points_per_meter', 10) * 5;
        if ($data['score'] > $expectedMaxScore) {
            $errors[] = "Score suspiciously high for distance";
        }

        // Validate track selection consistency
        if (isset($data['selected_track_indices']) && count($data['selected_track_indices']) > 3) {
            if (!$data['is_endless']) {
                $errors[] = "Too many track selections for finite run";
            }
        }

        return [
            'is_valid' => count($errors) === 0,
            'errors' => $errors,
            'is_suspicious' => count($errors) > 0,
        ];
    }
}
