<?php

namespace App\Services;

use App\Models\Run;
use App\Models\User;

class AnonymousDataMergeService
{
    public function merge(string $deviceId, User $user): int
    {
        return Run::where('device_id', $deviceId)
            ->whereNull('user_id')
            ->update(['user_id' => $user->id]);
    }
}

