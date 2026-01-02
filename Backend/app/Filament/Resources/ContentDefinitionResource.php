<?php

namespace App\Filament\Resources;

use App\Filament\Resources\ContentDefinitionResource\Pages;
use App\Models\ContentDefinition;
use Filament\Actions;
use Filament\Forms;
use Filament\Schemas\Schema;
use Filament\Resources\Resource;
use Filament\Tables;
use Filament\Tables\Table;

class ContentDefinitionResource extends Resource
{
    protected static ?string $model = ContentDefinition::class;

    protected static string | \BackedEnum | null $navigationIcon = 'heroicon-o-cube';

    public static function form(Schema $form): Schema
    {
        return $form
            ->components([
                Forms\Components\TextInput::make('content_id')->required()->unique(ignoreRecord: true),
                Forms\Components\TextInput::make('name')->required(),
                Forms\Components\Select::make('type')->options([
                    'track_piece' => 'Track Piece',
                    'powerup' => 'Power Up',
                    'obstacle' => 'Obstacle',
                    'collectible' => 'Collectible',
                ])->required(),
                Forms\Components\KeyValue::make('properties'),
            ]);
    }

    public static function table(Table $table): Table
    {
        return $table
            ->columns([
                Tables\Columns\TextColumn::make('content_id')->searchable()->sortable(),
                Tables\Columns\TextColumn::make('name')->searchable()->sortable(),
                Tables\Columns\TextColumn::make('type')->sortable(),
                Tables\Columns\TextColumn::make('updated_at')->dateTime()->sortable(),
            ])
            ->filters([
                Tables\Filters\SelectFilter::make('type')->options([
                    'track_piece' => 'Track Piece',
                    'powerup' => 'Power Up',
                    'obstacle' => 'Obstacle',
                    'collectible' => 'Collectible',
                ]),
            ])
            ->actions([
                Actions\ViewAction::make(),
                Actions\EditAction::make(),
            ]);
    }

    public static function getPages(): array
    {
        return [
            'index' => Pages\ListContentDefinitions::route('/'),
            'create' => Pages\CreateContentDefinition::route('/create'),
            'view' => Pages\ViewContentDefinition::route('/{record}'),
            'edit' => Pages\EditContentDefinition::route('/{record}/edit'),
        ];
    }
}
