<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Player;
use App\Models\User;
use App\Services\AnonymousDataMergeService;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Str;
use Laravel\Socialite\Facades\Socialite;

class AuthController extends Controller
{
    public function __construct(
        protected AnonymousDataMergeService $mergeService
    ) {
    }

    public function register(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'name' => 'required|string|max:255',
            'email' => 'required|string|email|max:255|unique:users',
            'password' => 'required|string|min:8',
            'username' => 'required|string|max:255|unique:players',
            'display_name' => 'required|string|max:255',
            'device_id' => 'sometimes|string', // Optional device_id for merging anonymous data
        ]);

        $user = User::create([
            'name' => $validated['name'],
            'email' => $validated['email'],
            'password' => Hash::make($validated['password']),
        ]);

        $player = Player::create([
            'user_id' => $user->id,
            'username' => $validated['username'],
            'display_name' => $validated['display_name'],
        ]);

        // Initialize default currency
        $player->addCurrency('coins', 0);

        // Merge anonymous data if device_id provided
        $mergeSummary = null;
        if (isset($validated['device_id']) && !empty($validated['device_id'])) {
            try {
                $mergeSummary = $this->mergeService->mergeDeviceDataToUser($validated['device_id'], $user);
            } catch (\Exception $e) {
                // Log error but don't fail registration
                \Log::warning("Failed to merge anonymous data during registration: " . $e->getMessage());
            }
        }

        $token = $user->createToken('auth-token')->plainTextToken;

        return response()->json([
            'user' => $user->load('player'),
            'token' => $token,
            'merge_summary' => $mergeSummary,
        ], 201);
    }

    public function login(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'email' => 'required|email',
            'password' => 'required',
        ]);

        if (! Auth::attempt($validated)) {
            return response()->json(['message' => 'Invalid credentials'], 401);
        }

        $user = Auth::user();

        if ($user->isBanned()) {
            return response()->json(['message' => 'Account is banned'], 403);
        }

        $token = $user->createToken('auth-token')->plainTextToken;

        return response()->json([
            'user' => $user->load('player'),
            'token' => $token,
        ]);
    }

    public function logout(Request $request): JsonResponse
    {
        $request->user()->currentAccessToken()->delete();

        return response()->json(['message' => 'Logged out successfully']);
    }

    public function refresh(Request $request): JsonResponse
    {
        $user = $request->user();
        $request->user()->currentAccessToken()->delete();
        $token = $user->createToken('auth-token')->plainTextToken;

        return response()->json([
            'user' => $user->load('player'),
            'token' => $token,
        ]);
    }

    public function user(Request $request): JsonResponse
    {
        return response()->json($request->user()->load('player'));
    }

    public function deviceAuth(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'device_id' => 'required|string',
            'username' => 'required|string|max:255|unique:players',
            'display_name' => 'required|string|max:255',
        ]);

        $isNewUser = !User::where('device_id', $validated['device_id'])->exists();

        $user = User::firstOrCreate(
            ['device_id' => $validated['device_id']],
            [
                'name' => $validated['display_name'],
                'email' => $validated['device_id'].'@device.local',
                'password' => Hash::make(Str::random(32)),
            ]
        );

        if (! $user->player) {
            $player = Player::create([
                'user_id' => $user->id,
                'username' => $validated['username'],
                'display_name' => $validated['display_name'],
            ]);

            $player->addCurrency('coins', 0);
        }

        // Merge anonymous data if this is a new user (first device auth)
        $mergeSummary = null;
        if ($isNewUser) {
            try {
                $mergeSummary = $this->mergeService->mergeDeviceDataToUser($validated['device_id'], $user);
            } catch (\Exception $e) {
                \Log::warning("Failed to merge anonymous data during device auth: " . $e->getMessage());
            }
        }

        $token = $user->createToken('device-token')->plainTextToken;

        return response()->json([
            'user' => $user->load('player'),
            'token' => $token,
            'merge_summary' => $mergeSummary,
        ], 201);
    }

    public function socialLogin(Request $request, string $provider): JsonResponse
    {
        $validated = $request->validate([
            'access_token' => 'required|string',
        ]);

        if (! in_array($provider, ['google', 'facebook'])) {
            return response()->json(['message' => 'Invalid provider'], 400);
        }

        try {
            $socialUser = Socialite::driver($provider)
                ->stateless()
                ->userFromToken($validated['access_token']);

            $providerIdField = $provider.'_id';

            $user = User::firstOrCreate(
                [$providerIdField => $socialUser->getId()],
                [
                    'name' => $socialUser->getName(),
                    'email' => $socialUser->getEmail() ?? $socialUser->getId().'@'.$provider.'.local',
                    'password' => Hash::make(Str::random(32)),
                ]
            );

            if (! $user->player) {
                $username = Str::slug($socialUser->getName() ?? $socialUser->getId());
                $player = Player::create([
                    'user_id' => $user->id,
                    'username' => $username,
                    'display_name' => $socialUser->getName() ?? $username,
                ]);

                $player->addCurrency('coins', 0);
            }

            $token = $user->createToken('social-token')->plainTextToken;

            return response()->json([
                'user' => $user->load('player'),
                'token' => $token,
            ]);
        } catch (\Exception $e) {
            return response()->json(['message' => 'Social authentication failed'], 401);
        }
    }

    /**
     * Merge anonymous device data to authenticated user
     * POST /api/auth/merge-device-data
     */
    public function mergeDeviceData(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'device_id' => 'required|string',
        ]);

        $user = $request->user();

        try {
            $mergeSummary = $this->mergeService->mergeDeviceDataToUser($validated['device_id'], $user);

            return response()->json([
                'message' => 'Device data merged successfully',
                'merge_summary' => $mergeSummary,
            ]);
        } catch (\Exception $e) {
            return response()->json([
                'message' => 'Failed to merge device data',
                'error' => $e->getMessage(),
            ], 500);
        }
    }
}
