<?php

namespace App\Filament\Resources\RunResource\Pages;

use App\Filament\Resources\RunResource;
use Filament\Actions;
use Filament\Resources\Pages\EditRecord;

class EditRun extends EditRecord
{
    protected static string $resource = RunResource::class;

    protected function getHeaderActions(): array
    {
        return [
            Actions\ViewAction::make(),
            Actions\DeleteAction::make(),
        ];
    }
}

