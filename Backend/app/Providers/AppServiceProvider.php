<?php

namespace App\Providers;

use Illuminate\Support\ServiceProvider;

class AppServiceProvider extends ServiceProvider
{
    /**
     * Register any application services.
     */
    public function register(): void
    {
        //
    }

    /**
     * Bootstrap any application services.
     */
    public function boot(): void
    {
        // Configure rate limiting for auth endpoints
        \Illuminate\Support\Facades\RateLimiter::for('auth', function ($request) {
            $endpoint = $request->path();
            
            // Login and register: 5 attempts per minute
            if (str_contains($endpoint, 'auth/login') || str_contains($endpoint, 'auth/register')) {
                return \Illuminate\Cache\RateLimiting\Limit::perMinute(5)->by($request->ip());
            }
            
            // Device auth: 10 attempts per minute
            if (str_contains($endpoint, 'auth/device')) {
                return \Illuminate\Cache\RateLimiting\Limit::perMinute(10)->by($request->ip());
            }
            
            // Token refresh: 20 attempts per minute
            if (str_contains($endpoint, 'auth/refresh')) {
                return \Illuminate\Cache\RateLimiting\Limit::perMinute(20)->by($request->user()?->id ?? $request->ip());
            }
            
            // Default: 60 attempts per minute
            return \Illuminate\Cache\RateLimiting\Limit::perMinute(60)->by($request->ip());
        });
    }
}
