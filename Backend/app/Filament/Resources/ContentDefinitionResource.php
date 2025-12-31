<?php

namespace App\Filament\Resources;

use App\Filament\Resources\ContentDefinitionResource\Pages;
use App\Models\ContentDefinition;
use Filament\Actions\ViewAction;
use Filament\Resources\Resource;
use Filament\Schemas\Schema;
use Filament\Tables;
use Filament\Tables\Table;

class ContentDefinitionResource extends Resource
{
    protected static ?string $model = ContentDefinition::class;

    protected static ?string $navigationLabel = 'Content Definitions';

    protected static ?int $navigationSort = 1;

    public static function getNavigationIcon(): string
    {
        return 'heroicon-o-cube';
    }

    public static function form(Schema $schema): Schema
    {
        return $schema;
    }

    public static function table(Table $table): Table
    {
        return $table
            ->columns([
                Tables\Columns\TextColumn::make('contentVersion.version')
                    ->label('Version')
                    ->sortable()
                    ->searchable(),
                Tables\Columns\TextColumn::make('content_type')
                    ->label('Type')
                    ->badge()
                    ->color(fn (string $state): string => match ($state) {
                        'track_piece' => 'info',
                        'obstacle' => 'warning',
                        'powerup' => 'success',
                        'collectible' => 'primary',
                        default => 'gray',
                    })
                    ->sortable()
                    ->searchable(),
                Tables\Columns\TextColumn::make('content_id')
                    ->label('ID')
                    ->searchable(),
                Tables\Columns\TextColumn::make('name')
                    ->searchable()
                    ->sortable(),
                Tables\Columns\IconColumn::make('is_active')
                    ->boolean()
                    ->sortable(),
                Tables\Columns\TextColumn::make('created_at')
                    ->dateTime()
                    ->sortable()
                    ->toggleable(isToggledHiddenByDefault: true),
            ])
            ->filters([
                Tables\Filters\SelectFilter::make('content_type')
                    ->options([
                        'track_piece' => 'Track Piece',
                        'obstacle' => 'Obstacle',
                        'powerup' => 'PowerUp',
                        'collectible' => 'Collectible',
                    ]),
                Tables\Filters\SelectFilter::make('content_version_id')
                    ->relationship('contentVersion', 'version')
                    ->label('Version'),
                Tables\Filters\TernaryFilter::make('is_active')
                    ->label('Active'),
            ])
            ->actions([
                ViewAction::make(),
            ])
            ->bulkActions([
                // No bulk actions for read-only view
            ])
            ->defaultSort('content_version_id', 'desc')
            ->modifyQueryUsing(function ($query) {
                // Show current active version by default, but allow filtering
                return $query->whereHas('contentVersion', function ($q) {
                    $q->where('is_active', true);
                });
            });
    }

    public static function infolist(Schema $schema): Schema
    {
        return $schema;
    }

    public static function getRelations(): array
    {
        return [
            //
        ];
    }

    public static function getPages(): array
    {
        return [
            'index' => Pages\ListContentDefinitions::route('/'),
            'view' => Pages\ViewContentDefinition::route('/{record}'),
        ];
    }

    public static function canCreate(): bool
    {
        return false; // Content is imported from UE, not created manually
    }

    public static function canEdit($record): bool
    {
        return false; // Content is imported from UE, not edited manually
    }

    public static function canDelete($record): bool
    {
        return false; // Content is managed through versions
    }
}

