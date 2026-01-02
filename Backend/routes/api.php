<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\AuthController;
use App\Http\Controllers\Api\RunController;
use App\Http\Controllers\Api\ContentController;

Route::post('/login', [AuthController::class, 'login']);
Route::post('/register', [AuthController::class, 'register']);

Route::middleware('auth:sanctum')->group(function () {
    Route::post('/logout', [AuthController::class, 'logout']);
    Route::get('/me', [AuthController::class, 'me']);
    Route::post('/merge-anonymous', [AuthController::class, 'mergeAnonymous']);
});

Route::prefix('runs')->group(function () {
    Route::post('/start', [RunController::class, 'start']);
    Route::get('/{seed_id}/tier/{tier}', [RunController::class, 'getTierTracks']);
    Route::post('/{seed_id}/select-track', [RunController::class, 'selectTrack']);
    Route::get('/{seed_id}/shop/{tier}/{track_index}/{shop_index}', [RunController::class, 'getShopItems']);
    Route::get('/{seed_id}/boss-rewards/{tier}', [RunController::class, 'getBossRewards']);
    Route::post('/', [RunController::class, 'store']);
});

Route::prefix('content')->group(function () {
    Route::get('/latest', [ContentController::class, 'latest']);
    Route::post('/import', [ContentController::class, 'import']);
});
