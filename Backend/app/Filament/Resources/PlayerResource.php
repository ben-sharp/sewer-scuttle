<?php

namespace App\Filament\Resources;

use App\Filament\Resources\PlayerResource\Pages;
use App\Models\Player;
use Filament\Actions;
use Filament\Forms;
use Filament\Schemas\Schema;
use Filament\Resources\Resource;
use Filament\Tables;
use Filament\Tables\Table;

class PlayerResource extends Resource
{
    protected static ?string $model = Player::class;

    protected static string | \BackedEnum | null $navigationIcon = 'heroicon-o-user-circle';

    public static function form(Schema $form): Schema
    {
        return $form
            ->components([
                Forms\Components\Select::make('user_id')
                    ->relationship('user', 'email')
                    ->placeholder('Guest Player'),
                Forms\Components\TextInput::make('device_id')
                    ->maxLength(255),
                Forms\Components\TextInput::make('username')
                    ->maxLength(255),
                Forms\Components\TextInput::make('display_name')
                    ->maxLength(255),
                Forms\Components\TextInput::make('player_class')
                    ->maxLength(255),
                Forms\Components\TextInput::make('total_coins')
                    ->numeric()
                    ->default(0),
                Forms\Components\TextInput::make('total_distance')
                    ->numeric()
                    ->default(0),
                Forms\Components\TextInput::make('total_runs')
                    ->numeric()
                    ->default(0),
                Forms\Components\TextInput::make('best_score')
                    ->numeric()
                    ->default(0),
                Forms\Components\KeyValue::make('customizations'),
            ]);
    }

    public static function table(Table $table): Table
    {
        return $table
            ->columns([
                Tables\Columns\TextColumn::make('status')
                    ->badge()
                    ->getStateUsing(fn ($record) => $record->user_id ? 'Registered' : 'Guest')
                    ->color(fn ($state) => $state === 'Registered' ? 'success' : 'warning'),
                Tables\Columns\TextColumn::make('username')->searchable()->sortable(),
                Tables\Columns\TextColumn::make('display_name')->searchable(),
                Tables\Columns\TextColumn::make('device_id')->searchable()->toggleable(isToggledHiddenByDefault: true),
                Tables\Columns\TextColumn::make('user.email')->label('User Email')->searchable(),
                Tables\Columns\TextColumn::make('player_class')->sortable(),
                Tables\Columns\TextColumn::make('total_coins')->numeric()->sortable(),
                Tables\Columns\TextColumn::make('best_score')->numeric()->sortable(),
                Tables\Columns\TextColumn::make('created_at')->dateTime()->sortable(),
            ])
            ->filters([
                Tables\Filters\Filter::make('guest_players')
                    ->query(fn ($query) => $query->whereNull('user_id')),
                Tables\Filters\Filter::make('registered_players')
                    ->query(fn ($query) => $query->whereNotNull('user_id')),
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
            'index' => Pages\ListPlayers::route('/'),
            'create' => Pages\CreatePlayer::route('/create'),
            'view' => Pages\ViewPlayer::route('/{record}'),
            'edit' => Pages\EditPlayer::route('/{record}/edit'),
        ];
    }
}
