// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpPath.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "HttpResultCallback.h"
#include "HttpRouteHandle.h"

class IHttpRouter;
class FHttpServerModule;

/**
 * Manages the HTTP server for the Revolt plugin
 * Handles routing, request processing, and JSON responses
 */
class FHttpServerManager
{
public:
	
	/** Get singleton instance */
	static FHttpServerManager& Get();

	/** Start the HTTP server */
	bool StartServer();

	/** Stop the HTTP server */
	void StopServer();

	/** Check if server is running */
	bool IsServerRunning() const { return bIsRunning; }

	/** Get the server port */
	int32 GetServerPort() const { return ServerPort; }

private:

	/** Private constructor for singleton */
	FHttpServerManager();

	/** Destructor */
	~FHttpServerManager();

	/** Register all API routes */
	void RegisterRoutes();

	/** Unregister all API routes */
	void UnregisterRoutes();

	// ========================================================================
	// Route Handlers
	// ========================================================================

	/** GET /api/status - Server status */
	bool HandleStatus(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/blueprints - List all blueprints */
	bool HandleGetBlueprints(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/blueprints/{name} - Get specific blueprint */
	bool HandleGetBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/levels - List all levels */
	bool HandleGetLevels(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/levels/{name}/actors - Get actors in level */
	bool HandleGetLevelActors(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/assets - Query assets */
	bool HandleGetAssets(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/characters - Get character blueprints */
	bool HandleGetCharacters(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/weapons - Get weapon blueprints */
	bool HandleGetWeapons(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/ai - Get AI configurations */
	bool HandleGetAI(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/search - Search across all assets */
	bool HandleSearch(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// ========================================================================
	// Edit Endpoints
	// ========================================================================

	/** PATCH /api/blueprints/{name}/properties - Edit blueprint properties */
	bool HandleEditBlueprintProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/blueprints/{name}/properties/validate - Validate property changes */
	bool HandleValidateProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/blueprints/{name}/duplicate - Duplicate blueprint */
	bool HandleDuplicateBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/blueprints - Create new blueprint */
	bool HandleCreateBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/data-assets - Create new data asset */
	bool HandleCreateDataAsset(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/levels/{name}/actors - Spawn actor in level */
	bool HandleSpawnActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/levels - Create new level */
	bool HandleCreateLevel(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/levels/{name}/duplicate - Duplicate level */
	bool HandleDuplicateLevel(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/levels/{name}/environment - Add environment to level */
	bool HandleAddEnvironment(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** PATCH /api/blueprints/bulk - Bulk edit blueprints */
	bool HandleBulkEdit(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/batch - Execute batch operations */
	bool HandleBatch(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// ========================================================================
	// Data Asset Endpoints
	// ========================================================================

	/** GET /api/data-assets - List all Data Assets */
	bool HandleGetDataAssets(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** GET /api/data-assets/{name} - Get specific Data Asset */
	bool HandleGetDataAsset(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** PATCH /api/data-assets/{name}/properties - Edit Data Asset properties */
	bool HandleEditDataAssetProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** POST /api/data-assets/{name}/properties/validate - Validate Data Asset property changes */
	bool HandleValidateDataAssetProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// ========================================================================
	// Utility Functions
	// ========================================================================

	/** Parse JSON from request body */
	bool ParseRequestBody(const FHttpServerRequest& Request, TSharedPtr<FJsonObject>& OutJson);

	/** Convert array of JSON objects to array of JSON values */
	TArray<TSharedPtr<FJsonValue>> ConvertToJsonValueArray(const TArray<TSharedPtr<FJsonObject>>& Objects);

	/** Create JSON success response */
	TUniquePtr<FHttpServerResponse> CreateJsonResponse(const TSharedPtr<FJsonObject>& JsonObject, EHttpServerResponseCodes ResponseCode = EHttpServerResponseCodes::Ok);

	/** Create JSON error response */
	TUniquePtr<FHttpServerResponse> CreateErrorResponse(const FString& ErrorMessage, EHttpServerResponseCodes ResponseCode = EHttpServerResponseCodes::ServerError);

	/** Extract path parameter from request */
	FString GetPathParameter(const FHttpServerRequest& Request, const FString& ParamName);

	/** Log request */
	void LogRequest(const FHttpServerRequest& Request);

private:

	/** Singleton instance */
	static FHttpServerManager* Instance;

	/** HTTP router */
	TSharedPtr<IHttpRouter> HttpRouter;

	/** Route handles for cleanup */
	TArray<FHttpRouteHandle> RouteHandles;

	/** Server running state */
	bool bIsRunning;

	/** Server port */
	int32 ServerPort;
};

