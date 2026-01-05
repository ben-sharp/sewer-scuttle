<?php

namespace App\Filament\Resources;

use App\Filament\Resources\RunResource\Pages;
use App\Models\Run;
use Filament\Actions;
use Filament\Forms;
use Filament\Schemas\Schema;
use Filament\Resources\Resource;
use Filament\Tables;
use Filament\Tables\Table;

class RunResource extends Resource
{
    protected static ?string $model = Run::class;

    protected static string | \BackedEnum | null $navigationIcon = 'heroicon-o-flag';

    public static function form(Schema $form): Schema
    {
        return $form
            ->components([
                Forms\Components\TextInput::make('seed_id')->required(),
                Forms\Components\TextInput::make('player_class'),
                Forms\Components\Select::make('player_id')->relationship('player', 'username'),
                Forms\Components\TextInput::make('device_id'),
                Forms\Components\TextInput::make('score')->numeric()->required(),
                Forms\Components\TextInput::make('distance')->numeric()->required(),
                Forms\Components\Toggle::make('is_complete'),
                Forms\Components\Toggle::make('is_endless'),
                Forms\Components\Toggle::make('is_suspicious'),
            ]);
    }

    public static function table(Table $table): Table
    {
        return $table
            ->columns([
                Tables\Columns\TextColumn::make('created_at')->dateTime()->sortable(),
                Tables\Columns\TextColumn::make('player.username')->label('Player'),
                Tables\Columns\TextColumn::make('player_class')->sortable()->searchable(),
                Tables\Columns\TextColumn::make('score')->numeric()->sortable(),
                Tables\Columns\TextColumn::make('distance')->numeric()->sortable(),
                Tables\Columns\IconColumn::make('is_complete')->boolean(),
                Tables\Columns\IconColumn::make('is_endless')->boolean(),
                Tables\Columns\IconColumn::make('is_suspicious')->boolean(),
            ])
            ->filters([
                Tables\Filters\SelectFilter::make('player_class')
                    ->options([
                        'Rabbit' => 'Rabbit',
                        'Vanilla' => 'Vanilla',
                        'Bear' => 'Bear',
                        'Fox' => 'Fox',
                        'Wolf' => 'Wolf',
                        'Boar' => 'Boar',
                        'Deer' => 'Deer',
                    ]),
                Tables\Filters\Filter::make('complete')->query(fn ($query) => $query->where('is_complete', true)),
                Tables\Filters\Filter::make('endless')->query(fn ($query) => $query->where('is_endless', true)),
                Tables\Filters\Filter::make('suspicious')->query(fn ($query) => $query->where('is_suspicious', true)),
            ])
            ->actions([
                Actions\ViewAction::make(),
                Actions\EditAction::make(),
            ])
            ->bulkActions([
                Actions\BulkActionGroup::make([
                    Actions\DeleteBulkAction::make(),
                ]),
            ]);
    }

    public static function getPages(): array
    {
        return [
            'index' => Pages\ListRuns::route('/'),
            'create' => Pages\CreateRun::route('/create'),
            'view' => Pages\ViewRun::route('/{record}'),
            'edit' => Pages\EditRun::route('/{record}/edit'),
        ];
    }
}
