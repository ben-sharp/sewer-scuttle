// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UBlueprint;

/**
 * Filter criteria for bulk operations
 */
struct FBulkFilter
{
	FString Type;           // "weapon", "character", "ai"
	FString ParentClass;    // Parent class name
	FString NamePattern;    // Wildcard pattern for name matching
	FString PathContains;   // Path must contain this string

	FBulkFilter() = default;
};

/**
 * Handles bulk editing operations on multiple assets
 */
class FBulkEditHandler
{
public:

	/**
	 * Filter blueprints based on criteria
	 * @param Filter Filter criteria
	 * @return Array of matching blueprints
	 */
	static TArray<UBlueprint*> FilterBlueprints(const FBulkFilter& Filter);

	/**
	 * Edit properties on multiple blueprints
	 * @param Blueprints Array of blueprints to edit
	 * @param Properties Map of property names to values (supports operators like *1.5, +10, -5)
	 * @param OutResults Array of result objects for each blueprint
	 * @return True if all edits successful
	 */
	static bool BulkEditProperties(const TArray<UBlueprint*>& Blueprints, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutResults);

	/**
	 * Parse a property value expression (supports operators like *1.5, +10, -5)
	 * @param Expression Value expression
	 * @param CurrentValue Current property value
	 * @return Calculated new value
	 */
	static FString ParseValueExpression(const FString& Expression, const FString& CurrentValue);

	/**
	 * Check if a blueprint matches the filter
	 * @param Blueprint Blueprint to check
	 * @param Filter Filter criteria
	 * @return True if matches
	 */
	static bool MatchesFilter(const UBlueprint* Blueprint, const FBulkFilter& Filter);

private:

	/**
	 * Convert array of JSON objects to array of JSON values (helper for HttpServerManager)
	 * @param Objects Array of JSON objects
	 * @return Array of JSON values
	 */
	static TArray<TSharedPtr<FJsonValue>> ConvertToJsonValueArray(const TArray<TSharedPtr<FJsonObject>>& Objects);

	/**
	 * Apply arithmetic operation to numeric value
	 * @param Operator Operator (*,+,-,/)
	 * @param CurrentValue Current value
	 * @param Operand Value to apply
	 * @return Result of operation
	 */
	static double ApplyOperation(const FString& Operator, double CurrentValue, double Operand);
};

