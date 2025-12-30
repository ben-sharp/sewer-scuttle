// Copyright Epic Games, Inc. All Rights Reserved.

#include "HttpServerManager.h"
#include "RevoltSettings.h"
#include "QueryOptions.h"
#include "BlueprintQueryHandler.h"
#include "LevelQueryHandler.h"
#include "AssetQueryHandler.h"
#include "GameplayQueryHandler.h"
#include "BlueprintEditHandler.h"
#include "ValidationManager.h"
#include "BackupManager.h"
#include "TransactionManager.h"
#include "BlueprintFactoryHandler.h"
#include "DataAssetFactoryHandler.h"
#include "LevelFactoryHandler.h"
#include "LevelEditHandler.h"
#include "BulkEditHandler.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "HttpPath.h"
#include "HttpServerRequest.h"
#include "HttpServerConstants.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"

FHttpServerManager* FHttpServerManager::Instance = nullptr;

FHttpServerManager::FHttpServerManager()
	: bIsRunning(false)
	, ServerPort(8080)
{
}

FHttpServerManager::~FHttpServerManager()
{
	StopServer();
}

FHttpServerManager& FHttpServerManager::Get()
{
	if (!Instance)
	{
		Instance = new FHttpServerManager();
	}
	return *Instance;
}

bool FHttpServerManager::StartServer()
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: HTTP server is already running"));
		return true;
	}

	// Get settings
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings)
	{
		ServerPort = Settings->ServerPort;
	}

	// Check if HTTPServer module is available
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("HTTPServer")))
	{
		if (!FModuleManager::Get().ModuleExists(TEXT("HTTPServer")))
		{
			UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: HTTPServer module not available. Plugin requires UE5 with HTTPServer support."));
			return false;
		}
		FModuleManager::Get().LoadModule(TEXT("HTTPServer"));
	}

	// Get HTTP server module
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	
	// Create router
	HttpRouter = HttpServerModule.GetHttpRouter(ServerPort);
	if (!HttpRouter)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create HTTP router on port %d"), ServerPort);
		return false;
	}

	// Register routes
	RegisterRoutes();

	// Start listening
	HttpServerModule.StartAllListeners();

	bIsRunning = true;
	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: HTTP server started on port %d"), ServerPort);
	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: API available at http://localhost:%d/api/"), ServerPort);

	return true;
}

void FHttpServerManager::StopServer()
{
	if (!bIsRunning)
	{
		return;
	}

	// Unregister routes
	UnregisterRoutes();

	// Stop server
	if (FModuleManager::Get().IsModuleLoaded("HTTPServer"))
	{
	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	HttpServerModule.StopAllListeners();
	}

	HttpRouter.Reset();
	bIsRunning = false;

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: HTTP server stopped"));
}

void FHttpServerManager::RegisterRoutes()
{
	if (!HttpRouter)
	{
		return;
	}

	// Status endpoint
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/status")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleStatus)
	));

	// Blueprint endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetBlueprints)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints/:name")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetBlueprint)
	));

	// Level endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetLevels)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels/:name/actors")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetLevelActors)
	));

	// Asset endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/assets")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetAssets)
	));

	// Gameplay endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/characters")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetCharacters)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/weapons")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetWeapons)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/ai")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetAI)
	));

	// Search endpoint
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/search")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleSearch)
	));

	// ========================================================================
	// Edit Endpoints
	// ========================================================================

	// Blueprint edit endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints/:name/properties")),
		EHttpServerRequestVerbs::VERB_PATCH,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleEditBlueprintProperties)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints/:name/properties/validate")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleValidateProperties)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints/:name/duplicate")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleDuplicateBlueprint)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleCreateBlueprint)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/data-assets")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetDataAssets)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/data-assets")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleCreateDataAsset)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/data-assets/:name")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleGetDataAsset)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/data-assets/:name/properties")),
		EHttpServerRequestVerbs::VERB_PATCH,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleEditDataAssetProperties)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/data-assets/:name/properties/validate")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleValidateDataAssetProperties)
	));

	// Level edit endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels/:name/actors")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleSpawnActor)
	));

	// Level creation/duplication endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleCreateLevel)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels/:name/duplicate")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleDuplicateLevel)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/levels/:name/environment")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleAddEnvironment)
	));

	// Bulk edit endpoints
	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/blueprints/bulk")),
		EHttpServerRequestVerbs::VERB_PATCH,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleBulkEdit)
	));

	RouteHandles.Add(HttpRouter->BindRoute(
		FHttpPath(TEXT("/api/batch")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateRaw(this, &FHttpServerManager::HandleBatch)
	));

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Registered %d API routes"), RouteHandles.Num());
}

void FHttpServerManager::UnregisterRoutes()
{
	if (HttpRouter)
	{
		for (const FHttpRouteHandle& Handle : RouteHandles)
		{
			HttpRouter->UnbindRoute(Handle);
		}
	}
	RouteHandles.Empty();
}

// ============================================================================
// Route Handlers
// ============================================================================

bool FHttpServerManager::HandleStatus(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("status"), TEXT("running"));
	JsonObject->SetNumberField(TEXT("port"), ServerPort);
	JsonObject->SetStringField(TEXT("version"), TEXT("2.3.7"));
	JsonObject->SetStringField(TEXT("plugin"), TEXT("RevoltUnrealPlugin"));

	OnComplete(CreateJsonResponse(JsonObject));
	return true;
}

bool FHttpServerManager::HandleGetBlueprints(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse query options
	FString QueryString = Request.QueryParams.Contains(TEXT("query")) ? Request.QueryParams[TEXT("query")] : TEXT("");
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(Request.QueryParams.Contains(TEXT("")) ? Request.QueryParams[TEXT("")] : TEXT(""));

	// Get blueprints
	TSharedPtr<FJsonObject> Result = FBlueprintQueryHandler::GetAllBlueprints(Options);

	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString BlueprintName = GetPathParameter(Request, TEXT("name"));
	if (BlueprintName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Blueprint name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse query options from query string
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	// Get specific blueprint
	TSharedPtr<FJsonObject> Result = FBlueprintQueryHandler::GetBlueprint(BlueprintName, Options);

	if (!Result)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Blueprint '%s' not found"), *BlueprintName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetLevels(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	TSharedPtr<FJsonObject> Result = FLevelQueryHandler::GetAllLevels();
	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetLevelActors(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString LevelName = GetPathParameter(Request, TEXT("name"));
	if (LevelName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Level name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse query options
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	// Get level actors
	TSharedPtr<FJsonObject> Result = FLevelQueryHandler::GetLevelActors(LevelName, Options);

	if (!Result)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Level '%s' not found or could not be loaded"), *LevelName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetAssets(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString AssetType = Request.QueryParams.Contains(TEXT("type")) ? Request.QueryParams[TEXT("type")] : TEXT("");
	
	TSharedPtr<FJsonObject> Result = FAssetQueryHandler::QueryAssets(AssetType);
	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetCharacters(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse query options
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	TSharedPtr<FJsonObject> Result = FGameplayQueryHandler::GetCharacters(Options);
	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetWeapons(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse query options
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	TSharedPtr<FJsonObject> Result = FGameplayQueryHandler::GetWeapons(Options);
	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetAI(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse query options
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	TSharedPtr<FJsonObject> Result = FGameplayQueryHandler::GetAIConfigurations(Options);
	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleSearch(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString SearchQuery = Request.QueryParams.Contains(TEXT("q")) ? Request.QueryParams[TEXT("q")] : TEXT("");
	
	if (SearchQuery.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Search query 'q' parameter required"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	TSharedPtr<FJsonObject> Result = FAssetQueryHandler::SearchAssets(SearchQuery);
	OnComplete(CreateJsonResponse(Result));
	return true;
}

// ============================================================================
// Edit Endpoint Handlers
// ============================================================================

bool FHttpServerManager::HandleEditBlueprintProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString BlueprintName = GetPathParameter(Request, TEXT("name"));
	if (BlueprintName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Blueprint name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON in request body"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check for confirmation
	bool bConfirm = false;
	RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm);
	if (!bConfirm)
	{
		OnComplete(CreateErrorResponse(TEXT("This operation requires 'confirm': true"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get properties to edit
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (!RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		OnComplete(CreateErrorResponse(TEXT("No properties field in request"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find the blueprint
	UBlueprint* Blueprint = FBlueprintEditHandler::FindBlueprint(BlueprintName);
	if (!Blueprint)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Blueprint '%s' not found"), *BlueprintName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Validate blueprint is editable
	FString ValidationError;
	if (!FValidationManager::ValidateBlueprintEditable(Blueprint, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Create backup if requested
	bool bCreateBackup = true;
	RequestJson->TryGetBoolField(TEXT("create_backup"), bCreateBackup);
	
	FString BackupPath;
	if (bCreateBackup)
	{
		BackupPath = FBackupManager::CreateBlueprintBackup(Blueprint);
		if (BackupPath.IsEmpty())
		{
			OnComplete(CreateErrorResponse(TEXT("Failed to create backup"), EHttpServerResponseCodes::ServerError));
			return true;
		}
	}

	// Convert JSON properties to map
	TMap<FString, FString> PropertiesToEdit;
	for (const auto& Pair : (*PropertiesObj)->Values)
	{
		FString ValueStr;
		if (Pair.Value->TryGetString(ValueStr))
		{
			PropertiesToEdit.Add(Pair.Key, ValueStr);
		}
		else if (Pair.Value->Type == EJson::Number)
		{
			PropertiesToEdit.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
		}
		else if (Pair.Value->Type == EJson::Boolean)
		{
			PropertiesToEdit.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
		}
	}

	// Edit properties
	TArray<TSharedPtr<FJsonObject>> Changes;
	bool bSuccess = FBlueprintEditHandler::EditBlueprintProperties(Blueprint, PropertiesToEdit, Changes);

	if (!bSuccess)
	{
		// Restore backup if edit failed
		if (!BackupPath.IsEmpty())
		{
			FBackupManager::RestoreBackup(BackupPath);
		}
		OnComplete(CreateErrorResponse(TEXT("Failed to edit properties"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Compile if requested
	bool bCompile = true;
	RequestJson->TryGetBoolField(TEXT("compile"), bCompile);
	
	if (bCompile)
	{
		if (!FBlueprintEditHandler::CompileBlueprint(Blueprint))
		{
			// Compilation failed, restore backup
			if (!BackupPath.IsEmpty())
			{
				FBackupManager::RestoreBackup(BackupPath);
			}
			OnComplete(CreateErrorResponse(TEXT("Blueprint compilation failed"), EHttpServerResponseCodes::ServerError));
			return true;
		}
	}

	// Save blueprint
	if (!FBlueprintEditHandler::SaveBlueprint(Blueprint))
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to save blueprint"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("blueprint"), Blueprint->GetName());
	Response->SetArrayField(TEXT("changes"), ConvertToJsonValueArray(Changes));
	if (!BackupPath.IsEmpty())
	{
		Response->SetStringField(TEXT("backup_path"), BackupPath);
	}
	Response->SetStringField(TEXT("compilation_status"), bCompile ? TEXT("success") : TEXT("skipped"));

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleValidateProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString BlueprintName = GetPathParameter(Request, TEXT("name"));
	
	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find blueprint
	UBlueprint* Blueprint = FBlueprintEditHandler::FindBlueprint(BlueprintName);
	if (!Blueprint)
	{
		OnComplete(CreateErrorResponse(TEXT("Blueprint not found"), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Validate each property
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (!RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		OnComplete(CreateErrorResponse(TEXT("No properties field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	TArray<TSharedPtr<FJsonValue>> ValidationResults;
	for (const auto& Pair : (*PropertiesObj)->Values)
	{
		TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetStringField(TEXT("property"), Pair.Key);

		FProperty* Property = Blueprint->GeneratedClass->FindPropertyByName(*Pair.Key);
		if (!Property)
		{
			Result->SetBoolField(TEXT("valid"), false);
			Result->SetStringField(TEXT("error"), TEXT("Property not found"));
		}
		else
		{
			FString ValueStr;
			if (Pair.Value->Type == EJson::Number)
			{
				ValueStr = FString::SanitizeFloat(Pair.Value->AsNumber());
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				ValueStr = Pair.Value->AsBool() ? TEXT("true") : TEXT("false");
			}
			else
			{
				Pair.Value->TryGetString(ValueStr);
			}
			
			FString ValidationError;
			bool bValid = FValidationManager::ValidatePropertyValue(Property, ValueStr, ValidationError);
			
			Result->SetBoolField(TEXT("valid"), bValid);
			if (!bValid)
			{
				Result->SetStringField(TEXT("error"), ValidationError);
			}
		}

		ValidationResults.Add(MakeShared<FJsonValueObject>(Result));
	}

	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetArrayField(TEXT("validation_results"), ValidationResults);

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleDuplicateBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString BlueprintName = GetPathParameter(Request, TEXT("name"));
	if (BlueprintName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Blueprint name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get new name
	FString NewName;
	if (!RequestJson->TryGetStringField(TEXT("new_name"), NewName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'new_name' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate new name
	FString ValidationError;
	if (!FValidationManager::ValidateBlueprintName(NewName, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find source blueprint
	UBlueprint* SourceBlueprint = FBlueprintEditHandler::FindBlueprint(BlueprintName);
	if (!SourceBlueprint)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Source blueprint '%s' not found"), *BlueprintName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Get optional new path
	FString NewPath;
	RequestJson->TryGetStringField(TEXT("new_path"), NewPath);

	// Duplicate the blueprint
	UBlueprint* DuplicatedBlueprint = FBlueprintFactoryHandler::DuplicateBlueprint(SourceBlueprint, NewName, NewPath);
	if (!DuplicatedBlueprint)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to duplicate blueprint"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Apply property overrides if provided
	const TSharedPtr<FJsonObject>* PropertyOverrides;
	if (RequestJson->TryGetObjectField(TEXT("property_overrides"), PropertyOverrides))
	{
		TMap<FString, FString> PropertiesToEdit;
		for (const auto& Pair : (*PropertyOverrides)->Values)
		{
			FString ValueStr;
			if (Pair.Value->TryGetString(ValueStr))
			{
				PropertiesToEdit.Add(Pair.Key, ValueStr);
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				PropertiesToEdit.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				PropertiesToEdit.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
			}
		}

		TArray<TSharedPtr<FJsonObject>> Changes;
		FBlueprintEditHandler::EditBlueprintProperties(DuplicatedBlueprint, PropertiesToEdit, Changes);
		FBlueprintEditHandler::CompileBlueprint(DuplicatedBlueprint);
		FBlueprintEditHandler::SaveBlueprint(DuplicatedBlueprint);
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("original"), SourceBlueprint->GetName());
	Response->SetStringField(TEXT("duplicate"), DuplicatedBlueprint->GetName());
	Response->SetStringField(TEXT("path"), DuplicatedBlueprint->GetPathName());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleCreateBlueprint(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get blueprint name
	FString BlueprintName;
	if (!RequestJson->TryGetStringField(TEXT("name"), BlueprintName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'name' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate name
	FString ValidationError;
	if (!FValidationManager::ValidateBlueprintName(BlueprintName, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get parent class
	FString ParentClassName;
	if (!RequestJson->TryGetStringField(TEXT("parent_class"), ParentClassName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'parent_class' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find parent class
	UClass* ParentClass = FBlueprintFactoryHandler::FindClass(ParentClassName);
	if (!ParentClass)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Parent class '%s' not found"), *ParentClassName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Get package path
	FString PackagePath;
	if (!RequestJson->TryGetStringField(TEXT("location"), PackagePath))
	{
		// Default to /Game/
		PackagePath = TEXT("/Game/");
	}

	// Validate path
	if (!FValidationManager::ValidateAssetPath(PackagePath, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Create the blueprint
	UBlueprint* NewBlueprint = FBlueprintFactoryHandler::CreateBlueprint(ParentClass, BlueprintName, PackagePath);
	if (!NewBlueprint)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to create blueprint"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Apply initial properties if provided
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		TMap<FString, FString> PropertiesToSet;
		for (const auto& Pair : (*PropertiesObj)->Values)
		{
			FString ValueStr;
			if (Pair.Value->TryGetString(ValueStr))
			{
				PropertiesToSet.Add(Pair.Key, ValueStr);
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				PropertiesToSet.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				PropertiesToSet.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
			}
		}

		TArray<TSharedPtr<FJsonObject>> Changes;
		FBlueprintEditHandler::EditBlueprintProperties(NewBlueprint, PropertiesToSet, Changes);
		FBlueprintEditHandler::CompileBlueprint(NewBlueprint);
		FBlueprintEditHandler::SaveBlueprint(NewBlueprint);
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("blueprint_name"), NewBlueprint->GetName());
	Response->SetStringField(TEXT("blueprint_path"), NewBlueprint->GetPathName());
	Response->SetStringField(TEXT("created_at"), FDateTime::Now().ToString());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleCreateDataAsset(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check confirmation
	bool bConfirm = false;
	RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm);
	if (!bConfirm)
	{
		OnComplete(CreateErrorResponse(TEXT("This operation requires 'confirm': true"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get asset name
	FString AssetName;
	if (!RequestJson->TryGetStringField(TEXT("name"), AssetName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'name' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate name (basic check)
	FString ValidationError;
	if (AssetName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Asset name cannot be empty"), EHttpServerResponseCodes::BadRequest));
		return true;
	}
	if (AssetName.Contains(TEXT(" ")) || AssetName.Contains(TEXT(".")))
	{
		OnComplete(CreateErrorResponse(TEXT("Asset name cannot contain spaces or dots"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get DataAsset class
	FString ClassName;
	if (!RequestJson->TryGetStringField(TEXT("class"), ClassName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'class' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find class
	UClass* DataAssetClass = FDataAssetFactoryHandler::FindDataAssetClass(ClassName);
	if (!DataAssetClass)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Data Asset class '%s' not found"), *ClassName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Get package path
	FString PackagePath;
	if (!RequestJson->TryGetStringField(TEXT("location"), PackagePath))
	{
		// Default to /Game/Data/
		PackagePath = TEXT("/Game/Data/");
	}

	// Validate path
	if (!FValidationManager::ValidateAssetPath(PackagePath, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Create the Data Asset
	UDataAsset* NewDataAsset = FDataAssetFactoryHandler::CreateDataAsset(DataAssetClass, AssetName, PackagePath);
	if (!NewDataAsset)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to create Data Asset"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Apply initial properties if provided
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		bool bPropertiesChanged = false;
		for (const auto& Pair : (*PropertiesObj)->Values)
		{
			FString ValueStr;
			if (Pair.Value->TryGetString(ValueStr))
			{
				// Keep as string
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				ValueStr = FString::SanitizeFloat(Pair.Value->AsNumber());
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				ValueStr = Pair.Value->AsBool() ? TEXT("true") : TEXT("false");
			}

			FProperty* Property = NewDataAsset->GetClass()->FindPropertyByName(*Pair.Key);
			if (Property)
			{
				if (FBlueprintEditHandler::SetPropertyValue(NewDataAsset, Property, ValueStr))
				{
					bPropertiesChanged = true;
				}
			}
		}

		if (bPropertiesChanged)
		{
			// Save again if properties changed
			FDataAssetFactoryHandler::SaveDataAsset(NewDataAsset);
		}
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("asset_name"), NewDataAsset->GetName());
	Response->SetStringField(TEXT("asset_path"), NewDataAsset->GetPathName());
	Response->SetStringField(TEXT("created_at"), FDateTime::Now().ToString());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleCreateLevel(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check confirmation
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings && Settings->bRequireConfirmation)
	{
		bool bConfirm = false;
		if (!RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm) || !bConfirm)
		{
			OnComplete(CreateErrorResponse(TEXT("Confirmation required. Set 'confirm': true in request"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// Get level name
	FString LevelName;
	if (!RequestJson->TryGetStringField(TEXT("name"), LevelName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'name' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate name
	FString ValidationError;
	if (!FLevelFactoryHandler::ValidateLevelName(LevelName, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get package path
	FString PackagePath;
	if (!RequestJson->TryGetStringField(TEXT("location"), PackagePath))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'location' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate path
	if (!FValidationManager::ValidateAssetPath(PackagePath, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Create the level
	UWorld* NewLevel = FLevelFactoryHandler::CreateLevel(LevelName, PackagePath);
	if (!NewLevel)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to create level"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("level_name"), NewLevel->GetName());
	Response->SetStringField(TEXT("level_path"), NewLevel->GetPathName());
	Response->SetStringField(TEXT("created_at"), FDateTime::Now().ToString());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleDuplicateLevel(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString SourceLevelName = GetPathParameter(Request, TEXT("name"));
	if (SourceLevelName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Source level name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check confirmation
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings && Settings->bRequireConfirmation)
	{
		bool bConfirm = false;
		if (!RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm) || !bConfirm)
		{
			OnComplete(CreateErrorResponse(TEXT("Confirmation required. Set 'confirm': true in request"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// Get new level name
	FString NewLevelName;
	if (!RequestJson->TryGetStringField(TEXT("new_name"), NewLevelName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'new_name' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Validate new name
	FString ValidationError;
	if (!FLevelFactoryHandler::ValidateLevelName(NewLevelName, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Load source level
	UWorld* SourceLevel = FLevelEditHandler::LoadLevelForEditing(SourceLevelName);
	if (!SourceLevel)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Source level '%s' not found"), *SourceLevelName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Create backup of source level if enabled
	FString BackupPath;
	if (Settings && Settings->bAutoBackup)
	{
		BackupPath = FBackupManager::CreateLevelBackup(SourceLevel);
		if (BackupPath.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Failed to create backup for level '%s'"), *SourceLevelName);
		}
	}

	// Get target path (optional)
	FString TargetPath;
	RequestJson->TryGetStringField(TEXT("location"), TargetPath);

	// Validate target path if provided
	if (!TargetPath.IsEmpty() && !FValidationManager::ValidateAssetPath(TargetPath, ValidationError))
	{
		OnComplete(CreateErrorResponse(ValidationError, EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Duplicate the level
	UWorld* DuplicatedLevel = FLevelFactoryHandler::DuplicateLevel(SourceLevel, NewLevelName, TargetPath);
	if (!DuplicatedLevel)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to duplicate level"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("original_level"), SourceLevelName);
	Response->SetStringField(TEXT("new_level"), DuplicatedLevel->GetName());
	Response->SetStringField(TEXT("new_path"), DuplicatedLevel->GetPathName());
	if (!BackupPath.IsEmpty())
	{
		Response->SetStringField(TEXT("backup_path"), BackupPath);
	}
	Response->SetStringField(TEXT("duplicated_at"), FDateTime::Now().ToString());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleAddEnvironment(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString LevelName = GetPathParameter(Request, TEXT("name"));

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check confirmation
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings && Settings->bRequireConfirmation)
	{
		bool bConfirm = false;
		if (!RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm) || !bConfirm)
		{
			OnComplete(CreateErrorResponse(TEXT("Confirmation required. Set 'confirm': true in request"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// Load the level for editing
	UWorld* Level = FLevelEditHandler::LoadLevelForEditing(LevelName);
	if (!Level)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Level '%s' not found"), *LevelName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Create backup if enabled
	FString BackupPath;
	if (Settings && Settings->bAutoBackup)
	{
		BackupPath = FBackupManager::CreateLevelBackup(Level);
		if (BackupPath.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Failed to create backup for level '%s'"), *LevelName);
		}
	}

	// Get environment type
	FString EnvironmentType;
	RequestJson->TryGetStringField(TEXT("type"), EnvironmentType);

	TArray<TSharedPtr<FJsonValue>> SpawnedActors;

	// Add environment based on type
	if (EnvironmentType == TEXT("skybox") || EnvironmentType.IsEmpty())
	{
		// Add sky sphere
		FString SkySphereClassName = TEXT("BP_SkySphere");
		RequestJson->TryGetStringField(TEXT("sky_sphere_class"), SkySphereClassName);

		UClass* SkySphereClass = FBlueprintFactoryHandler::FindClass(SkySphereClassName);
		if (SkySphereClass)
		{
			AActor* SkySphere = FLevelEditHandler::SpawnActorInLevel(Level, SkySphereClass, FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector);
			if (SkySphere)
			{
				TSharedPtr<FJsonObject> ActorInfo = MakeShared<FJsonObject>();
				ActorInfo->SetStringField(TEXT("name"), SkySphere->GetName());
				ActorInfo->SetStringField(TEXT("class"), SkySphereClassName);
				SpawnedActors.Add(MakeShared<FJsonValueObject>(ActorInfo));
			}
		}

		// Add directional light using StaticClass()
		UClass* DirectionalLightClass = ADirectionalLight::StaticClass();
		if (DirectionalLightClass)
		{
			AActor* DirectionalLight = FLevelEditHandler::SpawnActorInLevel(Level, DirectionalLightClass, FVector(0, 0, 1000), FRotator(-45, 0, 0), FVector::OneVector);
			if (DirectionalLight)
			{
				TSharedPtr<FJsonObject> ActorInfo = MakeShared<FJsonObject>();
				ActorInfo->SetStringField(TEXT("name"), DirectionalLight->GetName());
				ActorInfo->SetStringField(TEXT("class"), TEXT("DirectionalLight"));
				SpawnedActors.Add(MakeShared<FJsonValueObject>(ActorInfo));
			}
		}
	}
	else if (EnvironmentType == TEXT("lighting"))
	{
		// Add basic lighting setup
		UClass* DirectionalLightClass = ADirectionalLight::StaticClass();
		if (DirectionalLightClass)
		{
			AActor* DirectionalLight = FLevelEditHandler::SpawnActorInLevel(Level, DirectionalLightClass, FVector(0, 0, 1000), FRotator(-45, 0, 0), FVector::OneVector);
			if (DirectionalLight)
			{
				TSharedPtr<FJsonObject> ActorInfo = MakeShared<FJsonObject>();
				ActorInfo->SetStringField(TEXT("name"), DirectionalLight->GetName());
				ActorInfo->SetStringField(TEXT("class"), TEXT("DirectionalLight"));
				SpawnedActors.Add(MakeShared<FJsonValueObject>(ActorInfo));
			}
		}

		// Add sky light
		UClass* SkyLightClass = ASkyLight::StaticClass();
		if (SkyLightClass)
		{
			AActor* SkyLight = FLevelEditHandler::SpawnActorInLevel(Level, SkyLightClass, FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector);
			if (SkyLight)
			{
				TSharedPtr<FJsonObject> ActorInfo = MakeShared<FJsonObject>();
				ActorInfo->SetStringField(TEXT("name"), SkyLight->GetName());
				ActorInfo->SetStringField(TEXT("class"), TEXT("SkyLight"));
				SpawnedActors.Add(MakeShared<FJsonValueObject>(ActorInfo));
			}
		}
	}

	// Save the level
	if (!FLevelEditHandler::SaveLevel(Level))
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to save level after adding environment"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("level_name"), Level->GetName());
	Response->SetStringField(TEXT("environment_type"), EnvironmentType.IsEmpty() ? TEXT("skybox") : EnvironmentType);
	Response->SetArrayField(TEXT("spawned_actors"), SpawnedActors);
	if (!BackupPath.IsEmpty())
	{
		Response->SetStringField(TEXT("backup_path"), BackupPath);
	}
	Response->SetStringField(TEXT("added_at"), FDateTime::Now().ToString());

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleSpawnActor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString LevelName = GetPathParameter(Request, TEXT("name"));
	if (LevelName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Level name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check confirmation
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings && Settings->bRequireConfirmation)
	{
		bool bConfirm = false;
		if (!RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm) || !bConfirm)
		{
			OnComplete(CreateErrorResponse(TEXT("Confirmation required. Set 'confirm': true in request"), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// Get actor class
	FString ActorClassName;
	if (!RequestJson->TryGetStringField(TEXT("actor_class"), ActorClassName))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'actor_class' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find actor class
	UClass* ActorClass = FBlueprintFactoryHandler::FindClass(ActorClassName);
	if (!ActorClass)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Actor class '%s' not found"), *ActorClassName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Load the level
	UWorld* World = FLevelEditHandler::LoadLevelForEditing(LevelName);
	if (!World)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Failed to load level '%s'"), *LevelName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Parse location
	FVector Location = FVector::ZeroVector;
	const TSharedPtr<FJsonObject>* LocationObj;
	if (RequestJson->TryGetObjectField(TEXT("location"), LocationObj))
	{
		(*LocationObj)->TryGetNumberField(TEXT("x"), Location.X);
		(*LocationObj)->TryGetNumberField(TEXT("y"), Location.Y);
		(*LocationObj)->TryGetNumberField(TEXT("z"), Location.Z);
	}

	// Parse rotation
	FRotator Rotation = FRotator::ZeroRotator;
	const TSharedPtr<FJsonObject>* RotationObj;
	if (RequestJson->TryGetObjectField(TEXT("rotation"), RotationObj))
	{
		(*RotationObj)->TryGetNumberField(TEXT("pitch"), Rotation.Pitch);
		(*RotationObj)->TryGetNumberField(TEXT("yaw"), Rotation.Yaw);
		(*RotationObj)->TryGetNumberField(TEXT("roll"), Rotation.Roll);
	}

	// Parse scale
	FVector Scale = FVector::OneVector;
	const TSharedPtr<FJsonObject>* ScaleObj;
	if (RequestJson->TryGetObjectField(TEXT("scale"), ScaleObj))
	{
		(*ScaleObj)->TryGetNumberField(TEXT("x"), Scale.X);
		(*ScaleObj)->TryGetNumberField(TEXT("y"), Scale.Y);
		(*ScaleObj)->TryGetNumberField(TEXT("z"), Scale.Z);
	}

	// Spawn the actor
	AActor* SpawnedActor = FLevelEditHandler::SpawnActorInLevel(World, ActorClass, Location, Rotation, Scale);
	if (!SpawnedActor)
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to spawn actor"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Configure properties if provided
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		TMap<FString, FString> Properties;
		for (const auto& Pair : (*PropertiesObj)->Values)
		{
			FString ValueStr;
			if (Pair.Value->TryGetString(ValueStr))
			{
				Properties.Add(Pair.Key, ValueStr);
			}
			else if (Pair.Value->Type == EJson::Number)
			{
				Properties.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				Properties.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
			}
		}

		FLevelEditHandler::ConfigureActorProperties(SpawnedActor, Properties);
	}

	// Add tags if provided
	const TArray<TSharedPtr<FJsonValue>>* TagsArray;
	if (RequestJson->TryGetArrayField(TEXT("tags"), TagsArray))
	{
		TArray<FString> Tags;
		for (const auto& TagValue : *TagsArray)
		{
			FString Tag;
			if (TagValue->TryGetString(Tag))
			{
				Tags.Add(Tag);
			}
		}
		FLevelEditHandler::AddActorTags(SpawnedActor, Tags);
	}

	// Save the level
	if (!FLevelEditHandler::SaveLevel(World))
	{
		OnComplete(CreateErrorResponse(TEXT("Failed to save level after spawning actor"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("actor_name"), SpawnedActor->GetName());
	Response->SetStringField(TEXT("actor_label"), SpawnedActor->GetActorLabel());
	
	TSharedPtr<FJsonObject> LocationJson = MakeShared<FJsonObject>();
	LocationJson->SetNumberField(TEXT("x"), Location.X);
	LocationJson->SetNumberField(TEXT("y"), Location.Y);
	LocationJson->SetNumberField(TEXT("z"), Location.Z);
	Response->SetObjectField(TEXT("location"), LocationJson);

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleBulkEdit(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check for confirmation
	bool bConfirm = false;
	RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm);
	if (!bConfirm)
	{
		OnComplete(CreateErrorResponse(TEXT("Bulk operations require 'confirm': true"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse filter
	const TSharedPtr<FJsonObject>* FilterObj;
	if (!RequestJson->TryGetObjectField(TEXT("filter"), FilterObj))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'filter' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	FBulkFilter Filter;
	(*FilterObj)->TryGetStringField(TEXT("type"), Filter.Type);
	(*FilterObj)->TryGetStringField(TEXT("parent_class"), Filter.ParentClass);
	(*FilterObj)->TryGetStringField(TEXT("name_pattern"), Filter.NamePattern);
	(*FilterObj)->TryGetStringField(TEXT("path_contains"), Filter.PathContains);

	// Get properties to edit
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (!RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'properties' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	TMap<FString, FString> Properties;
	for (const auto& Pair : (*PropertiesObj)->Values)
	{
		FString ValueStr;
		if (Pair.Value->TryGetString(ValueStr))
		{
			Properties.Add(Pair.Key, ValueStr);
		}
		else if (Pair.Value->Type == EJson::Number)
		{
			Properties.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
		}
		else if (Pair.Value->Type == EJson::Boolean)
		{
			Properties.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
		}
	}

	// Check for preview mode
	bool bPreview = false;
	RequestJson->TryGetBoolField(TEXT("preview"), bPreview);

	// Filter blueprints
	TArray<UBlueprint*> MatchingBlueprints = FBulkEditHandler::FilterBlueprints(Filter);

	if (bPreview)
	{
		// Preview mode - show what would be edited without executing
		TArray<TSharedPtr<FJsonValue>> PreviewArray;
		for (UBlueprint* BP : MatchingBlueprints)
		{
			TSharedPtr<FJsonObject> PreviewJson = MakeShared<FJsonObject>();
			PreviewJson->SetStringField(TEXT("blueprint"), BP->GetName());
			PreviewJson->SetStringField(TEXT("path"), BP->GetPathName());
			PreviewArray.Add(MakeShared<FJsonValueObject>(PreviewJson));
		}

		TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
		Response->SetBoolField(TEXT("success"), true);
		Response->SetBoolField(TEXT("preview"), true);
		Response->SetNumberField(TEXT("count"), MatchingBlueprints.Num());
		Response->SetArrayField(TEXT("affected_blueprints"), PreviewArray);

		OnComplete(CreateJsonResponse(Response));
		return true;
	}

	// Execute bulk edit
	TArray<TSharedPtr<FJsonObject>> Results;
	bool bSuccess = FBulkEditHandler::BulkEditProperties(MatchingBlueprints, Properties, Results);

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), bSuccess);
	Response->SetNumberField(TEXT("total_count"), MatchingBlueprints.Num());
	Response->SetNumberField(TEXT("success_count"), Results.Num());
	Response->SetArrayField(TEXT("results"), ConvertToJsonValueArray(Results));

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleBatch(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get operations array
	const TArray<TSharedPtr<FJsonValue>>* OperationsArray;
	if (!RequestJson->TryGetArrayField(TEXT("operations"), OperationsArray))
	{
		OnComplete(CreateErrorResponse(TEXT("Missing 'operations' field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check for atomic flag
	bool bAtomic = false;
	RequestJson->TryGetBoolField(TEXT("atomic"), bAtomic);

	// Start transaction if atomic
	if (bAtomic)
	{
		FTransactionManager::Get().BeginTransaction(TEXT("Batch Operations"));
	}

	TArray<TSharedPtr<FJsonValue>> Results;
	bool bAllSucceeded = true;

	for (const auto& OpValue : *OperationsArray)
	{
		const TSharedPtr<FJsonObject>* OpObj;
		if (!OpValue->TryGetObject(OpObj))
		{
			continue;
		}

		FString OpType;
		(*OpObj)->TryGetStringField(TEXT("type"), OpType);

		TSharedPtr<FJsonObject> OpResult = MakeShared<FJsonObject>();
		OpResult->SetStringField(TEXT("type"), OpType);

		// Execute based on operation type
		if (OpType == TEXT("edit_property"))
		{
			// Individual property edit
			FString Target;
			(*OpObj)->TryGetStringField(TEXT("target"), Target);
			
			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString PropertyName, Value, OldValue;
				(*OpObj)->TryGetStringField(TEXT("property"), PropertyName);
				(*OpObj)->TryGetStringField(TEXT("value"), Value);

				bool bSuccess = FBlueprintEditHandler::EditBlueprintProperty(BP, *PropertyName, Value, OldValue);
				OpResult->SetBoolField(TEXT("success"), bSuccess);
				
				if (!bSuccess && bAtomic)
				{
					bAllSucceeded = false;
					break;
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("add_variable"))
		{
			FString Target, Name, Type;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("name"), Name);
			(*OpObj)->TryGetStringField(TEXT("type"), Type);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				FString ResultName = FBlueprintEditHandler::AddBlueprintVariable(BP, Name, Type, Error);
				if (!ResultName.IsEmpty())
				{
					OpResult->SetBoolField(TEXT("success"), true);
					OpResult->SetStringField(TEXT("variable_name"), ResultName);
				}
				else
				{
					OpResult->SetBoolField(TEXT("success"), false);
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("add_function"))
		{
			FString Target, Name;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("name"), Name);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				FString ResultName = FBlueprintEditHandler::AddBlueprintFunction(BP, Name, Error);
				if (!ResultName.IsEmpty())
				{
					OpResult->SetBoolField(TEXT("success"), true);
					OpResult->SetStringField(TEXT("function_name"), ResultName);
				}
				else
				{
					OpResult->SetBoolField(TEXT("success"), false);
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("add_node"))
		{
			FString Target, GraphName, NodeClass;
			int32 X = 0, Y = 0;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("graph"), GraphName);
			(*OpObj)->TryGetStringField(TEXT("node_class"), NodeClass);
			(*OpObj)->TryGetNumberField(TEXT("x"), X);
			(*OpObj)->TryGetNumberField(TEXT("y"), Y);

			TMap<FString, FString> Properties;
			const TSharedPtr<FJsonObject>* PropsObj;
			if ((*OpObj)->TryGetObjectField(TEXT("properties"), PropsObj))
			{
				for (const auto& Pair : (*PropsObj)->Values)
				{
					FString Val;
					if (Pair.Value->TryGetString(Val)) Properties.Add(Pair.Key, Val);
				}
			}

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				UEdGraphNode* NewNode = FBlueprintEditHandler::AddGraphNode(BP, GraphName, NodeClass, X, Y, Properties, Error);
				if (NewNode)
				{
					OpResult->SetBoolField(TEXT("success"), true);
					OpResult->SetStringField(TEXT("node_name"), NewNode->GetName());
					OpResult->SetStringField(TEXT("node_guid"), NewNode->NodeGuid.ToString());
				}
				else
				{
					OpResult->SetBoolField(TEXT("success"), false);
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("connect_pins"))
		{
			FString Target, GraphName;
			FString NodeA, PinA, NodeB, PinB;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("graph"), GraphName);
			(*OpObj)->TryGetStringField(TEXT("node_a"), NodeA);
			(*OpObj)->TryGetStringField(TEXT("pin_a"), PinA);
			(*OpObj)->TryGetStringField(TEXT("node_b"), NodeB);
			(*OpObj)->TryGetStringField(TEXT("pin_b"), PinB);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				bool bConnected = FBlueprintEditHandler::ConnectNodePins(BP, GraphName, NodeA, PinA, NodeB, PinB, Error);
				OpResult->SetBoolField(TEXT("success"), bConnected);
				if (!bConnected)
				{
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("delete_node"))
		{
			FString Target, GraphName, NodeName;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("graph"), GraphName);
			(*OpObj)->TryGetStringField(TEXT("node"), NodeName);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				bool bDeleted = FBlueprintEditHandler::RemoveGraphNode(BP, GraphName, NodeName, Error);
				OpResult->SetBoolField(TEXT("success"), bDeleted);
				if (!bDeleted)
				{
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("add_component"))
		{
			FString Target, ComponentClass, ComponentName, ParentName;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("class"), ComponentClass);
			(*OpObj)->TryGetStringField(TEXT("name"), ComponentName);
			(*OpObj)->TryGetStringField(TEXT("parent"), ParentName);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				FString ResultName = FBlueprintEditHandler::AddComponent(BP, ComponentClass, ComponentName, ParentName, Error);
				if (!ResultName.IsEmpty())
				{
					OpResult->SetBoolField(TEXT("success"), true);
					OpResult->SetStringField(TEXT("component_name"), ResultName);
				}
				else
				{
					OpResult->SetBoolField(TEXT("success"), false);
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("remove_component"))
		{
			FString Target, ComponentName;
			(*OpObj)->TryGetStringField(TEXT("blueprint"), Target);
			(*OpObj)->TryGetStringField(TEXT("name"), ComponentName);

			UBlueprint* BP = FBlueprintEditHandler::FindBlueprint(Target);
			if (BP)
			{
				FString Error;
				bool bRemoved = FBlueprintEditHandler::RemoveComponent(BP, ComponentName, Error);
				OpResult->SetBoolField(TEXT("success"), bRemoved);
				if (!bRemoved)
				{
					OpResult->SetStringField(TEXT("error"), Error);
					if (bAtomic) { bAllSucceeded = false; break; }
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				OpResult->SetStringField(TEXT("error"), TEXT("Blueprint not found"));
				if (bAtomic) { bAllSucceeded = false; break; }
			}
		}
		else if (OpType == TEXT("spawn_actor"))
		{
			// Actor spawning
			FString LevelName, ActorClass;
			(*OpObj)->TryGetStringField(TEXT("level"), LevelName);
			(*OpObj)->TryGetStringField(TEXT("actor"), ActorClass);

			// Get location
			FVector Location = FVector::ZeroVector;
			const TSharedPtr<FJsonObject>* LocationObj;
			if ((*OpObj)->TryGetObjectField(TEXT("location"), LocationObj))
			{
				(*LocationObj)->TryGetNumberField(TEXT("x"), Location.X);
				(*LocationObj)->TryGetNumberField(TEXT("y"), Location.Y);
				(*LocationObj)->TryGetNumberField(TEXT("z"), Location.Z);
			}

			UWorld* World = FLevelEditHandler::LoadLevelForEditing(LevelName);
			UClass* Class = FBlueprintFactoryHandler::FindClass(ActorClass);

			if (World && Class)
			{
				AActor* Actor = FLevelEditHandler::SpawnActorInLevel(World, Class, Location, FRotator::ZeroRotator, FVector::OneVector);
				OpResult->SetBoolField(TEXT("success"), Actor != nullptr);
				
				if (!Actor && bAtomic)
				{
					bAllSucceeded = false;
					break;
				}
			}
			else
			{
				OpResult->SetBoolField(TEXT("success"), false);
				if (bAtomic)
				{
					bAllSucceeded = false;
					break;
				}
			}
		}

		Results.Add(MakeShared<FJsonValueObject>(OpResult));
	}

	// Handle transaction result
	if (bAtomic)
	{
		if (bAllSucceeded)
		{
			FTransactionManager::Get().EndTransaction();
		}
		else
		{
			FTransactionManager::Get().CancelTransaction();
		}
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), bAllSucceeded);
	Response->SetNumberField(TEXT("total"), OperationsArray->Num());
	Response->SetNumberField(TEXT("completed"), Results.Num());
	Response->SetArrayField(TEXT("results"), Results);

	OnComplete(CreateJsonResponse(Response));
	return true;
}

// ============================================================================
// Utility Functions
// ============================================================================

bool FHttpServerManager::ParseRequestBody(const FHttpServerRequest& Request, TSharedPtr<FJsonObject>& OutJson)
{
	if (Request.Body.Num() == 0)
	{
		return false;
	}

	// Convert body bytes to string
	FString BodyString;
	FFileHelper::BufferToString(BodyString, Request.Body.GetData(), Request.Body.Num());

	// Parse JSON
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyString);
	return FJsonSerializer::Deserialize(Reader, OutJson);
}

TArray<TSharedPtr<FJsonValue>> FHttpServerManager::ConvertToJsonValueArray(const TArray<TSharedPtr<FJsonObject>>& Objects)
{
	TArray<TSharedPtr<FJsonValue>> Values;
	for (const auto& Obj : Objects)
	{
		Values.Add(MakeShared<FJsonValueObject>(Obj));
	}
	return Values;
}

// ============================================================================
// Existing Utility Functions
// ============================================================================

TUniquePtr<FHttpServerResponse> FHttpServerManager::CreateJsonResponse(const TSharedPtr<FJsonObject>& JsonObject, EHttpServerResponseCodes ResponseCode)
{
	// Serialize JSON
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Create response
	TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(JsonString, TEXT("application/json"));
	Response->Code = ResponseCode;
	
	return Response;
}

TUniquePtr<FHttpServerResponse> FHttpServerManager::CreateErrorResponse(const FString& ErrorMessage, EHttpServerResponseCodes ResponseCode)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetBoolField(TEXT("success"), false);
	JsonObject->SetStringField(TEXT("error"), ErrorMessage);
	
	return CreateJsonResponse(JsonObject, ResponseCode);
}

FString FHttpServerManager::GetPathParameter(const FHttpServerRequest& Request, const FString& ParamName)
{
	if (Request.PathParams.Contains(ParamName))
	{
		return Request.PathParams[ParamName];
	}
	return FString();
}

void FHttpServerManager::LogRequest(const FHttpServerRequest& Request)
{
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	if (Settings && Settings->bEnableLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: %s %s"), *Request.RelativePath.GetPath(), Request.QueryParams.Num() > 0 ? TEXT("(with params)") : TEXT(""));
	}
}

// ============================================================================
// Data Asset Endpoint Handlers
// ============================================================================

bool FHttpServerManager::HandleGetDataAssets(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	// Parse query options
	FString QueryString = Request.QueryParams.Contains(TEXT("query")) ? Request.QueryParams[TEXT("query")] : TEXT("");
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	// Get class filter if specified
	UClass* DataAssetClass = nullptr;
	FString ClassFilter = Request.QueryParams.Contains(TEXT("class")) ? Request.QueryParams[TEXT("class")] : TEXT("");
	if (!ClassFilter.IsEmpty())
	{
		DataAssetClass = FDataAssetFactoryHandler::FindDataAssetClass(ClassFilter);
		if (!DataAssetClass)
		{
			OnComplete(CreateErrorResponse(FString::Printf(TEXT("Data Asset class '%s' not found"), *ClassFilter), EHttpServerResponseCodes::BadRequest));
			return true;
		}
	}

	// Get all data assets
	TSharedPtr<FJsonObject> Result = FDataAssetFactoryHandler::GetAllDataAssets(DataAssetClass, Options);

	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleGetDataAsset(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString DataAssetName = GetPathParameter(Request, TEXT("name"));
	if (DataAssetName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Data Asset name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse query options from query string
	FString QueryString;
	for (const auto& Param : Request.QueryParams)
	{
		if (!QueryString.IsEmpty())
		{
			QueryString += TEXT("&");
		}
		QueryString += Param.Key + TEXT("=") + Param.Value;
	}
	FQueryOptions Options = FQueryOptions::ParseFromQueryString(QueryString);

	// Get specific data asset
	TSharedPtr<FJsonObject> Result = FDataAssetFactoryHandler::GetDataAssetInfo(DataAssetName, Options);

	if (!Result)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Data Asset '%s' not found"), *DataAssetName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	OnComplete(CreateJsonResponse(Result));
	return true;
}

bool FHttpServerManager::HandleEditDataAssetProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString DataAssetName = GetPathParameter(Request, TEXT("name"));
	if (DataAssetName.IsEmpty())
	{
		OnComplete(CreateErrorResponse(TEXT("Data Asset name not provided"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON in request body"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Check for confirmation
	bool bConfirm = false;
	RequestJson->TryGetBoolField(TEXT("confirm"), bConfirm);
	if (!bConfirm)
	{
		OnComplete(CreateErrorResponse(TEXT("This operation requires 'confirm': true"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Get properties to edit
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (!RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		OnComplete(CreateErrorResponse(TEXT("No properties field in request"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find the data asset
	UDataAsset* DataAsset = FDataAssetFactoryHandler::FindDataAsset(DataAssetName);
	if (!DataAsset)
	{
		OnComplete(CreateErrorResponse(FString::Printf(TEXT("Data Asset '%s' not found"), *DataAssetName), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Create backup if requested
	bool bCreateBackup = true;
	RequestJson->TryGetBoolField(TEXT("create_backup"), bCreateBackup);

	FString BackupPath;
	if (bCreateBackup)
	{
		BackupPath = FBackupManager::CreateDataAssetBackup(DataAsset);
		if (BackupPath.IsEmpty())
		{
			OnComplete(CreateErrorResponse(TEXT("Failed to create backup"), EHttpServerResponseCodes::ServerError));
			return true;
		}
	}

	// Convert JSON properties to map
	TMap<FString, FString> PropertiesToEdit;
	for (const auto& Pair : (*PropertiesObj)->Values)
	{
		FString ValueStr;
		if (Pair.Value->TryGetString(ValueStr))
		{
			PropertiesToEdit.Add(Pair.Key, ValueStr);
		}
		else if (Pair.Value->Type == EJson::Number)
		{
			PropertiesToEdit.Add(Pair.Key, FString::SanitizeFloat(Pair.Value->AsNumber()));
		}
		else if (Pair.Value->Type == EJson::Boolean)
		{
			PropertiesToEdit.Add(Pair.Key, Pair.Value->AsBool() ? TEXT("true") : TEXT("false"));
		}
	}

	// Edit properties
	TArray<TSharedPtr<FJsonObject>> Changes;
	bool bSuccess = FDataAssetFactoryHandler::EditDataAssetProperties(DataAssetName, PropertiesToEdit, Changes);

	if (!bSuccess)
	{
		// Restore backup if edit failed
		if (!BackupPath.IsEmpty())
		{
			FBackupManager::RestoreBackup(BackupPath);
		}
		OnComplete(CreateErrorResponse(TEXT("Failed to edit properties"), EHttpServerResponseCodes::ServerError));
		return true;
	}

	// Build response
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("data_asset"), DataAsset->GetName());
	Response->SetArrayField(TEXT("changes"), ConvertToJsonValueArray(Changes));
	if (!BackupPath.IsEmpty())
	{
		Response->SetStringField(TEXT("backup_path"), BackupPath);
	}

	OnComplete(CreateJsonResponse(Response));
	return true;
}

bool FHttpServerManager::HandleValidateDataAssetProperties(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	LogRequest(Request);

	FString DataAssetName = GetPathParameter(Request, TEXT("name"));

	// Parse request body
	TSharedPtr<FJsonObject> RequestJson;
	if (!ParseRequestBody(Request, RequestJson))
	{
		OnComplete(CreateErrorResponse(TEXT("Invalid JSON"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	// Find data asset
	UDataAsset* DataAsset = FDataAssetFactoryHandler::FindDataAsset(DataAssetName);
	if (!DataAsset)
	{
		OnComplete(CreateErrorResponse(TEXT("Data Asset not found"), EHttpServerResponseCodes::NotFound));
		return true;
	}

	// Validate each property
	const TSharedPtr<FJsonObject>* PropertiesObj;
	if (!RequestJson->TryGetObjectField(TEXT("properties"), PropertiesObj))
	{
		OnComplete(CreateErrorResponse(TEXT("No properties field"), EHttpServerResponseCodes::BadRequest));
		return true;
	}

	TArray<TSharedPtr<FJsonValue>> ValidationResults;
	for (const auto& Pair : (*PropertiesObj)->Values)
	{
		TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetStringField(TEXT("property"), Pair.Key);

		FProperty* Property = DataAsset->GetClass()->FindPropertyByName(*Pair.Key);
		if (!Property)
		{
			Result->SetBoolField(TEXT("valid"), false);
			Result->SetStringField(TEXT("error"), TEXT("Property not found"));
		}
		else
		{
			FString ValueStr;
			if (Pair.Value->Type == EJson::Number)
			{
				ValueStr = FString::SanitizeFloat(Pair.Value->AsNumber());
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				ValueStr = Pair.Value->AsBool() ? TEXT("true") : TEXT("false");
			}
			else
			{
				Pair.Value->TryGetString(ValueStr);
			}

			FString ValidationError;
			bool bValid = FValidationManager::ValidatePropertyValue(Property, ValueStr, ValidationError);

			Result->SetBoolField(TEXT("valid"), bValid);
			if (!bValid)
			{
				Result->SetStringField(TEXT("error"), ValidationError);
			}
		}

		ValidationResults.Add(MakeShared<FJsonValueObject>(Result));
	}

	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
	Response->SetBoolField(TEXT("success"), true);
	Response->SetArrayField(TEXT("validation_results"), ValidationResults);

	OnComplete(CreateJsonResponse(Response));
	return true;
}

