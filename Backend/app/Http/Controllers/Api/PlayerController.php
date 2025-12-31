<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\PlayerCurrency;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class PlayerController extends Controller
{
    public function profile(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        return response()->json($player->load('currencies'));
    }

    public function updateProfile(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        $validated = $request->validate([
            'display_name' => 'sometimes|string|max:255',
            'player_class' => 'sometimes|nullable|string|max:255',
        ]);

        $player->update($validated);

        return response()->json($player->load('currencies'));
    }

    public function currency(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        return response()->json($player->currencies);
    }

    public function addCurrency(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'player_id' => 'required|exists:players,id',
            'currency_type' => 'required|string',
            'amount' => 'required|integer|min:1',
        ]);

        $player = \App\Models\Player::findOrFail($validated['player_id']);
        $player->addCurrency($validated['currency_type'], $validated['amount']);

        return response()->json([
            'message' => 'Currency added successfully',
            'currency' => $player->getCurrency($validated['currency_type']),
        ]);
    }

    public function customizations(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        return response()->json([
            'customizations' => $player->customizations ?? [],
        ]);
    }

    public function updateCustomizations(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        $validated = $request->validate([
            'customizations' => 'required|array',
        ]);

        $player->update(['customizations' => $validated['customizations']]);

        return response()->json([
            'customizations' => $player->customizations,
        ]);
    }
}
