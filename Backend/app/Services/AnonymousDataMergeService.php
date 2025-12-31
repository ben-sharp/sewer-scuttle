<?php

namespace App\Services;

use App\Models\Run;
use App\Models\User;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Log;

class AnonymousDataMergeService
{
    /**
     * Merge anonymous device data to authenticated user
     *
     * @param string $deviceId
     * @param User $user
     * @return array Merge summary
     */
    public function mergeDeviceDataToUser(string $deviceId, User $user): array
    {
        $summary = [
            'runs_merged' => 0,
            'profile_merged' => false,
            'unlocks_merged' => 0, // Future
        ];

        DB::beginTransaction();

        try {
            // Find all runs with device_id
            $runs = Run::where('device_id', $deviceId)->get();
            $summary['runs_merged'] = $runs->count();

            // Update runs: set player_id, clear device_id
            if ($runs->isNotEmpty()) {
                Run::where('device_id', $deviceId)
                    ->update([
                        'player_id' => $user->player->id,
                        'device_id' => null,
                    ]);
            }

            // TODO: Find anonymous profile (if exists)
            // TODO: Merge profile preferences into players.customizations

            // TODO: Find all unlocks with device_id
            // TODO: Update unlocks: set user_id, clear device_id

            // TODO: Delete anonymous profile record

            DB::commit();

            Log::info("Merged anonymous data for device {$deviceId} to user {$user->id}", $summary);

            return $summary;
        } catch (\Exception $e) {
            DB::rollBack();
            Log::error("Failed to merge anonymous data for device {$deviceId}: " . $e->getMessage());
            throw $e;
        }
    }
}

