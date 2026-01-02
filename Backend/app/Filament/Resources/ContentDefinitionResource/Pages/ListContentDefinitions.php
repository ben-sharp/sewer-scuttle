<?php

namespace App\Filament\Resources\ContentDefinitionResource\Pages;

use App\Filament\Resources\ContentDefinitionResource;
use Filament\Actions;
use Filament\Resources\Pages\ListRecords;

class ListContentDefinitions extends ListRecords
{
    protected static string $resource = ContentDefinitionResource::class;

    protected function getHeaderActions(): array
    {
        return [
            Actions\CreateAction::make(),
        ];
    }
}

