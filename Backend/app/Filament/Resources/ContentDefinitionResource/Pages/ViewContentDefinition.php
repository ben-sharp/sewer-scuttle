<?php

namespace App\Filament\Resources\ContentDefinitionResource\Pages;

use App\Filament\Resources\ContentDefinitionResource;
use Filament\Actions;
use Filament\Resources\Pages\ViewRecord;

class ViewContentDefinition extends ViewRecord
{
    protected static string $resource = ContentDefinitionResource::class;

    protected function getHeaderActions(): array
    {
        return [
            Actions\EditAction::make(),
        ];
    }
}

