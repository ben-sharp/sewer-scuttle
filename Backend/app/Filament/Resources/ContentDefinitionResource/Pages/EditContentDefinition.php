<?php

namespace App\Filament\Resources\ContentDefinitionResource\Pages;

use App\Filament\Resources\ContentDefinitionResource;
use Filament\Actions;
use Filament\Resources\Pages\EditRecord;

class EditContentDefinition extends EditRecord
{
    protected static string $resource = ContentDefinitionResource::class;

    protected function getHeaderActions(): array
    {
        return [
            Actions\ViewAction::make(),
            Actions\DeleteAction::make(),
        ];
    }
}

