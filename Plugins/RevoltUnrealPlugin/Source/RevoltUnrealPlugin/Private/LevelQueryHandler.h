// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QueryOptions.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

class UWorld;
class AActor;

/**
 * Handles querying level assets with on-demand loading
 */
class FLevelQueryHandler
{
public:

	/** Get all levels in the project */
	static TSharedPtr<FJsonObject> GetAllLevels();

	/** Get actors in a specific level (loads level on demand) */
	static TSharedPtr<FJsonObject> GetLevelActors(const FString& LevelName, const FQueryOptions& Options);

private:

	/** Load a level by name */
	static UWorld* LoadLevel(const FString& LevelName);

	/** Extract actor information to JSON */
	static TSharedPtr<FJsonObject> ExtractActorInfo(AActor* Actor, const FQueryOptions& Options);

	/** Extract components from an actor */
	static void ExtractComponents(AActor* Actor, TSharedPtr<FJsonObject>& JsonObject);

	/** Extract properties from an actor */
	static void ExtractActorProperties(AActor* Actor, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options);

	/** Get property value as string */
	static FString GetPropertyValueAsString(FProperty* Property, const void* ValuePtr);
};

