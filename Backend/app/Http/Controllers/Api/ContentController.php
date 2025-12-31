<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\ContentDefinition;
use App\Models\ContentVersion;
use App\Services\ContentImportService;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class ContentController extends Controller
{
    public function __construct(
        protected ContentImportService $importService
    ) {
    }

    /**
     * Import content from UE
     * POST /api/content/import
     */
    public function import(Request $request): JsonResponse
    {
        // Check if user is admin (or allow in non-production)
        $user = $request->user();
        if (!$user || (!$user->is_admin && app()->environment('production'))) {
            return response()->json(['message' => 'Unauthorized'], 403);
        }

        $validated = $request->validate([
            'version' => 'required|string',
            'definitions' => 'required|array',
            'definitions.*.type' => 'required|string',
            'definitions.*.id' => 'required|string',
            'definitions.*.name' => 'required|string',
            'definitions.*.properties' => 'required|array',
            'exported_at' => 'sometimes|date',
        ]);

        try {
            $contentVersion = $this->importService->import($validated);

            return response()->json([
                'message' => 'Content imported successfully',
                'version' => $contentVersion->version,
                'definitions_count' => $contentVersion->definitions()->count(),
            ], 201);
        } catch (\Exception $e) {
            return response()->json([
                'message' => 'Failed to import content',
                'error' => $e->getMessage(),
            ], 500);
        }
    }

    /**
     * List all content definitions
     * GET /api/content/definitions
     */
    public function index(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'type' => 'sometimes|string|in:track_piece,obstacle,powerup,collectible',
            'version' => 'sometimes|string',
        ]);

        $query = ContentDefinition::with('contentVersion');

        if (isset($validated['type'])) {
            $query->ofType($validated['type']);
        }

        if (isset($validated['version'])) {
            $query->whereHas('contentVersion', function ($q) use ($validated) {
                $q->where('version', $validated['version']);
            });
        } else {
            // Default to current active version
            $currentVersion = ContentVersion::current();
            if ($currentVersion) {
                $query->where('content_version_id', $currentVersion->id);
            }
        }

        $definitions = $query->active()->get();

        return response()->json($definitions);
    }

    /**
     * Get current active content version
     * GET /api/content/current
     */
    public function current(): JsonResponse
    {
        $version = ContentVersion::current();

        if (!$version) {
            return response()->json(['message' => 'No active content version'], 404);
        }

        return response()->json([
            'version' => $version->version,
            'exported_at' => $version->exported_at,
            'definitions_count' => $version->definitions()->count(),
        ]);
    }

    /**
     * Get content for specific version
     * GET /api/content/version/{version}
     */
    public function version(string $version): JsonResponse
    {
        $contentVersion = ContentVersion::where('version', $version)->first();

        if (!$contentVersion) {
            return response()->json(['message' => 'Content version not found'], 404);
        }

        $definitions = $contentVersion->definitions()->active()->get();

        return response()->json([
            'version' => $contentVersion->version,
            'exported_at' => $contentVersion->exported_at,
            'is_active' => $contentVersion->is_active,
            'definitions' => $definitions,
        ]);
    }
}
