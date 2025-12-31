<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Purchase;
use App\Models\StoreItem;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class StoreController extends Controller
{
    public function items(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'type' => 'sometimes|nullable|string',
            'active_only' => 'sometimes|boolean',
        ]);

        $query = StoreItem::query();

        if ($validated['active_only'] ?? true) {
            $query->where('is_active', true);
        }

        if (isset($validated['type'])) {
            $query->where('item_type', $validated['type']);
        }

        $items = $query->get();

        return response()->json($items);
    }

    public function item(Request $request, int $id): JsonResponse
    {
        $item = StoreItem::findOrFail($id);

        return response()->json($item);
    }

    public function purchase(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'item_id' => 'required|exists:store_items,id',
        ]);

        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        $item = StoreItem::findOrFail($validated['item_id']);

        if (! $item->is_active) {
            return response()->json(['message' => 'Item is not available for purchase'], 400);
        }

        $currency = $player->getCurrency($item->currency_type);

        if (! $currency || $currency->amount < $item->price) {
            return response()->json([
                'message' => 'Insufficient currency',
                'required' => $item->price,
                'available' => $currency?->amount ?? 0,
            ], 400);
        }

        // Check if item is already purchased (for non-consumables)
        if ($item->item_type !== 'consumable') {
            $existingPurchase = Purchase::where('player_id', $player->id)
                ->where('store_item_id', $item->id)
                ->exists();

            if ($existingPurchase) {
                return response()->json(['message' => 'Item already purchased'], 400);
            }
        }

        // Deduct currency
        $currency->decrement('amount', $item->price);

        // Create purchase record
        $purchase = Purchase::create([
            'player_id' => $player->id,
            'store_item_id' => $item->id,
            'currency_type' => $item->currency_type,
            'amount_paid' => $item->price,
        ]);

        return response()->json([
            'message' => 'Purchase successful',
            'purchase' => $purchase->load('storeItem'),
            'remaining_currency' => $currency->amount,
        ], 201);
    }

    public function purchases(Request $request): JsonResponse
    {
        $player = $request->user()->player;

        if (! $player) {
            return response()->json(['message' => 'Player profile not found'], 404);
        }

        $purchases = Purchase::where('player_id', $player->id)
            ->with('storeItem')
            ->orderBy('created_at', 'desc')
            ->get();

        return response()->json($purchases);
    }
}
