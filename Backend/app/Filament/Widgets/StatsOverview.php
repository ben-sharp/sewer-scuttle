<?php

namespace App\Filament\Widgets;

use App\Models\Player;
use App\Models\Purchase;
use App\Models\Run;
use App\Models\StoreItem;
use App\Models\User;
use Filament\Widgets\StatsOverviewWidget;
use Filament\Widgets\StatsOverviewWidget\Stat;

class StatsOverview extends StatsOverviewWidget
{
    protected function getStats(): array
    {
        return [
            Stat::make('Total Users', User::count())
                ->description('Registered users')
                ->descriptionIcon('heroicon-o-users')
                ->color('success'),
            Stat::make('Total Players', Player::count())
                ->description('Active players')
                ->descriptionIcon('heroicon-o-user-group')
                ->color('info'),
            Stat::make('Total Runs', Run::count())
                ->description('Completed runs')
                ->descriptionIcon('heroicon-o-play')
                ->color('warning'),
            Stat::make('Total Purchases', Purchase::count())
                ->description('Store purchases')
                ->descriptionIcon('heroicon-o-shopping-cart')
                ->color('success'),
            Stat::make('Store Items', StoreItem::where('is_active', true)->count())
                ->description('Active items')
                ->descriptionIcon('heroicon-o-shopping-bag')
                ->color('info'),
        ];
    }
}
