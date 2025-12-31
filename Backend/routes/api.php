<?php

use App\Http\Controllers\Api\AuthController;
use App\Http\Controllers\Api\ContentController;
use App\Http\Controllers\Api\LeaderboardController;
use App\Http\Controllers\Api\PlayerController;
use App\Http\Controllers\Api\RunController;
use App\Http\Controllers\Api\StoreController;
use Illuminate\Support\Facades\Route;

// Public routes with rate limiting
Route::post('/auth/register', [AuthController::class, 'register'])->middleware('throttle:auth');
Route::post('/auth/login', [AuthController::class, 'login'])->middleware('throttle:auth');
Route::post('/auth/device', [AuthController::class, 'deviceAuth'])->middleware('throttle:auth');
Route::post('/auth/social/{provider}', [AuthController::class, 'socialLogin'])->middleware('throttle:auth');

// Public run endpoints (for anonymous users)
Route::post('/runs/start', [RunController::class, 'start']); // Request seed (works for anonymous)
Route::post('/runs', [RunController::class, 'store']); // Submit run (works for anonymous)
Route::get('/runs/history', [RunController::class, 'history']); // Get history (works for anonymous with device_id)

// Protected routes
Route::middleware('auth:sanctum')->group(function () {
    // Authentication
    Route::post('/auth/logout', [AuthController::class, 'logout']);
    Route::post('/auth/refresh', [AuthController::class, 'refresh'])->middleware('throttle:auth');
    Route::get('/auth/user', [AuthController::class, 'user']);
    Route::post('/auth/merge-device-data', [AuthController::class, 'mergeDeviceData']);

    // Player
    Route::prefix('player')->group(function () {
        Route::get('/profile', [PlayerController::class, 'profile']);
        Route::put('/profile', [PlayerController::class, 'updateProfile']);
        Route::get('/currency', [PlayerController::class, 'currency']);
        Route::post('/currency/add', [PlayerController::class, 'addCurrency'])->middleware(\App\Http\Middleware\EnsureUserIsAdmin::class);
        Route::get('/customizations', [PlayerController::class, 'customizations']);
        Route::put('/customizations', [PlayerController::class, 'updateCustomizations']);
    });

    // Leaderboard
    Route::prefix('leaderboard')->group(function () {
        Route::get('/', [LeaderboardController::class, 'index']);
        Route::post('/submit', [LeaderboardController::class, 'submit']);
        Route::get('/player/{id}', [LeaderboardController::class, 'playerRank']);
    });

    // Store
    Route::prefix('store')->group(function () {
        Route::get('/items', [StoreController::class, 'items']);
        Route::get('/items/{id}', [StoreController::class, 'item']);
        Route::post('/purchase', [StoreController::class, 'purchase']);
        Route::get('/purchases', [StoreController::class, 'purchases']);
    });

    // Runs (authenticated - can also use public endpoints)
    Route::prefix('runs')->group(function () {
        Route::post('/end', [RunController::class, 'end']); // Legacy endpoint
        Route::get('/{id}', [RunController::class, 'show']);
    });

    // Content (admin only in production)
    Route::prefix('content')->group(function () {
        Route::post('/import', [ContentController::class, 'import']);
        Route::get('/definitions', [ContentController::class, 'index']);
        Route::get('/current', [ContentController::class, 'current']);
        Route::get('/version/{version}', [ContentController::class, 'version']);
    });
});

// Public content endpoints
Route::prefix('content')->group(function () {
    Route::get('/definitions', [ContentController::class, 'index']);
    Route::get('/current', [ContentController::class, 'current']);
    Route::get('/version/{version}', [ContentController::class, 'version']);
});

