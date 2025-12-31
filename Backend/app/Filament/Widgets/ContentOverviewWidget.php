<?php

namespace App\Filament\Widgets;

use App\Models\ContentDefinition;
use App\Models\ContentVersion;
use Filament\Widgets\StatsOverviewWidget;
use Filament\Widgets\StatsOverviewWidget\Stat;

class ContentOverviewWidget extends StatsOverviewWidget
{
    protected static ?int $sort = 2;

    protected function getStats(): array
    {
        $currentVersion = ContentVersion::current();
        
        if (!$currentVersion) {
            return [
                Stat::make('Content Version', 'Not Available')
                    ->description('No active content version')
                    ->descriptionIcon('heroicon-o-exclamation-triangle')
                    ->color('danger'),
            ];
        }

        $definitions = $currentVersion->definitions()->active()->get();
        $trackPieces = $definitions->where('content_type', 'track_piece')->count();
        $obstacles = $definitions->where('content_type', 'obstacle')->count();
        $powerups = $definitions->where('content_type', 'powerup')->count();
        $collectibles = $definitions->where('content_type', 'collectible')->count();

        return [
            Stat::make('Content Version', $currentVersion->version)
                ->description('Active version')
                ->descriptionIcon('heroicon-o-tag')
                ->color('success'),
            Stat::make('Track Pieces', $trackPieces)
                ->description('Available track pieces')
                ->descriptionIcon('heroicon-o-map')
                ->color('info'),
            Stat::make('Obstacles', $obstacles)
                ->description('Obstacle types')
                ->descriptionIcon('heroicon-o-shield-exclamation')
                ->color('warning'),
            Stat::make('Powerups', $powerups)
                ->description('Powerup types')
                ->descriptionIcon('heroicon-o-sparkles')
                ->color('success'),
            Stat::make('Collectibles', $collectibles)
                ->description('Collectible types')
                ->descriptionIcon('heroicon-o-currency-dollar')
                ->color('info'),
            Stat::make('Total Definitions', $definitions->count())
                ->description('All content definitions')
                ->descriptionIcon('heroicon-o-cube')
                ->color('primary'),
        ];
    }
}
