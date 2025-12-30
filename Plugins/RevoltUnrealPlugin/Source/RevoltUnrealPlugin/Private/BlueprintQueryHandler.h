// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QueryOptions.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

class UBlueprint;
class UEdGraph;
class UEdGraphNode;

/**
 * Handles querying Blueprint assets with full graph analysis
 */
class FBlueprintQueryHandler
{
public:

	/** Get all blueprints in the project */
	static TSharedPtr<FJsonObject> GetAllBlueprints(const FQueryOptions& Options);

	/** Get specific blueprint by name */
	static TSharedPtr<FJsonObject> GetBlueprint(const FString& BlueprintName, const FQueryOptions& Options);

	/** Extract blueprint information to JSON */
	static TSharedPtr<FJsonObject> ExtractBlueprintInfo(UBlueprint* Blueprint, const FQueryOptions& Options);

private:

	/** Extract basic blueprint info */
	static void ExtractBasicInfo(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject);

	/** Extract properties from blueprint */
	static void ExtractProperties(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options);

	/** Extract functions from blueprint */
	static void ExtractFunctions(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options);

	/** Extract graph nodes from blueprint */
	static void ExtractGraphNodes(UBlueprint* Blueprint, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options);

	/** Extract single graph information */
	static TSharedPtr<FJsonObject> ExtractGraph(UEdGraph* Graph, const FQueryOptions& Options);

	/** Extract node information */
	static TSharedPtr<FJsonObject> ExtractNode(UEdGraphNode* Node, const FQueryOptions& Options);

	/** Get property value as string */
	static FString GetPropertyValueAsString(FProperty* Property, const void* ValuePtr);
};

