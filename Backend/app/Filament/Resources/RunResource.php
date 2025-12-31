<?php

namespace App\Filament\Resources;

use App\Filament\Resources\RunResource\Pages;
use App\Models\Run;
use Filament\Actions\ViewAction;
use Filament\Resources\Resource;
use Filament\Schemas\Schema;
use Filament\Tables;
use Filament\Tables\Table;

class RunResource extends Resource
{
    protected static ?string $model = Run::class;

    protected static ?string $navigationLabel = 'Runs';

    protected static ?int $navigationSort = 2;

    public static function getNavigationIcon(): string
    {
        return 'heroicon-o-play';
    }

    public static function form(Schema $schema): Schema
    {
        return $schema;
    }

    public static function table(Table $table): Table
    {
        return $table
            ->columns([
                Tables\Columns\TextColumn::make('id')
                    ->label('ID')
                    ->sortable()
                    ->searchable(),
                Tables\Columns\TextColumn::make('player.username')
                    ->label('Player')
                    ->default('Anonymous')
                    ->sortable()
                    ->searchable(),
                Tables\Columns\TextColumn::make('device_id')
                    ->label('Device ID')
                    ->default('N/A')
                    ->searchable()
                    ->toggleable(),
                Tables\Columns\TextColumn::make('score')
                    ->label('Score')
                    ->numeric()
                    ->sortable()
                    ->searchable(),
                Tables\Columns\TextColumn::make('distance')
                    ->label('Distance (m)')
                    ->numeric()
                    ->sortable(),
                Tables\Columns\TextColumn::make('duration_seconds')
                    ->label('Duration')
                    ->formatStateUsing(fn ($state) => gmdate('H:i:s', $state))
                    ->sortable(),
                Tables\Columns\TextColumn::make('coins_collected')
                    ->label('Coins')
                    ->numeric()
                    ->sortable(),
                Tables\Columns\TextColumn::make('obstacles_hit')
                    ->label('Obstacles Hit')
                    ->numeric()
                    ->sortable(),
                Tables\Columns\TextColumn::make('powerups_used')
                    ->label('PowerUps')
                    ->numeric()
                    ->sortable(),
                Tables\Columns\TextColumn::make('track_pieces_spawned')
                    ->label('Track Pieces')
                    ->numeric()
                    ->sortable()
                    ->toggleable(),
                Tables\Columns\IconColumn::make('is_suspicious')
                    ->label('Suspicious')
                    ->boolean()
                    ->sortable(),
                Tables\Columns\TextColumn::make('seed_id')
                    ->label('Seed ID')
                    ->searchable()
                    ->toggleable(isToggledHiddenByDefault: true),
                Tables\Columns\TextColumn::make('started_at')
                    ->dateTime()
                    ->sortable(),
                Tables\Columns\TextColumn::make('completed_at')
                    ->dateTime()
                    ->sortable(),
            ])
            ->filters([
                Tables\Filters\SelectFilter::make('is_suspicious')
                    ->label('Suspicious')
                    ->options([
                        0 => 'Valid',
                        1 => 'Suspicious',
                    ]),
                Tables\Filters\Filter::make('anonymous')
                    ->label('Anonymous Runs')
                    ->query(fn ($query) => $query->anonymous()),
                Tables\Filters\Filter::make('authenticated')
                    ->label('Authenticated Runs')
                    ->query(fn ($query) => $query->authenticated()),
            ])
            ->actions([
                ViewAction::make(),
            ])
            ->bulkActions([
                // No bulk actions for now
            ])
            ->defaultSort('completed_at', 'desc');
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
            'index' => Pages\ListRuns::route('/'),
            'view' => Pages\ViewRun::route('/{record}'),
        ];
    }
}

