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
                ->description('All game runs')
                ->descriptionIcon('heroicon-o-play')
                ->color('primary'),
            Stat::make('Successful Runs', Run::where('is_complete', true)->count())
                ->description('Completed successfully')
                ->descriptionIcon('heroicon-o-check-circle')
                ->color('success'),
            Stat::make('Deaths', Run::where('is_complete', false)->count())
                ->description('Failed or died')
                ->descriptionIcon('heroicon-o-x-circle')
                ->color('danger'),
            Stat::make('Endless Runs Started', Run::where('is_endless', true)->count())
                ->description('Total endless runs')
                ->descriptionIcon('heroicon-o-arrow-path')
                ->color('warning'),
            Stat::make('Endless Runs Ended', Run::where('is_endless', true)->where('is_complete', true)->count())
                ->description('Endless runs with milestone')
                ->descriptionIcon('heroicon-o-stop-circle')
                ->color('info'),
            Stat::make('Total Purchases', Purchase::count())
                ->description('Store purchases')
                ->descriptionIcon('heroicon-o-shopping-cart')
                ->color('success'),
        ];
    }
}
