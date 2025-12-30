// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QueryOptions.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

/**
 * Handles specialized gameplay queries for characters, weapons, and AI
 * Project-specific queries tailored for the Bluedrake42Unreal project
 */
class FGameplayQueryHandler
{
public:

	/** Get all character blueprints with gameplay data */
	static TSharedPtr<FJsonObject> GetCharacters(const FQueryOptions& Options);

	/** Get all weapon blueprints with gameplay data */
	static TSharedPtr<FJsonObject> GetWeapons(const FQueryOptions& Options);

	/** Get AI configurations */
	static TSharedPtr<FJsonObject> GetAIConfigurations(const FQueryOptions& Options);

private:

	/** Check if a blueprint is a character type */
	static bool IsCharacterBlueprint(class UBlueprint* Blueprint);

	/** Check if a blueprint is a weapon type */
	static bool IsWeaponBlueprint(class UBlueprint* Blueprint);

	/** Check if a blueprint is an AI type */
	static bool IsAIBlueprint(class UBlueprint* Blueprint);

	/** Extract character-specific data */
	static TSharedPtr<FJsonObject> ExtractCharacterData(class UBlueprint* Blueprint, const FQueryOptions& Options);

	/** Extract weapon-specific data */
	static TSharedPtr<FJsonObject> ExtractWeaponData(class UBlueprint* Blueprint, const FQueryOptions& Options);

	/** Extract AI-specific data */
	static TSharedPtr<FJsonObject> ExtractAIData(class UBlueprint* Blueprint, const FQueryOptions& Options);

	/** Get property value from a class */
	static FString GetPropertyValue(UClass* Class, const FString& PropertyName);
};

