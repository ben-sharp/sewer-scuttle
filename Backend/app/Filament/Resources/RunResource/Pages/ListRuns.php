<?php

namespace App\Filament\Resources\RunResource\Pages;

use App\Filament\Resources\RunResource;
use Filament\Actions;
use Filament\Resources\Pages\ListRecords;

class ListRuns extends ListRecords
{
    protected static string $resource = RunResource::class;

    protected function getHeaderActions(): array
    {
        return [
            // No create action - runs are created by game clients
        ];
    }
}

