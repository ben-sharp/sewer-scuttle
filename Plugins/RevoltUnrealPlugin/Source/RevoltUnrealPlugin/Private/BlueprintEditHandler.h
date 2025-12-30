// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UBlueprint;
class FProperty;

/**
 * Handles editing operations for Blueprint assets
 * Provides property modification with safety features
 */
class FBlueprintEditHandler
{
public:

	/**
	 * Find a blueprint by name
	 * @param BlueprintName Name or partial name of blueprint
	 * @return Found blueprint or nullptr
	 */
	static UBlueprint* FindBlueprint(const FString& BlueprintName);

	/**
	 * Edit a single property on a blueprint
	 * @param Blueprint Target blueprint to edit
	 * @param PropertyName Name of property to modify
	 * @param Value New value as string
	 * @param OutOldValue Returns old value before change
	 * @return True if edit successful
	 */
	static bool EditBlueprintProperty(UBlueprint* Blueprint, const FName& PropertyName, const FString& Value, FString& OutOldValue);

	/**
	 * Edit multiple properties on a blueprint
	 * @param Blueprint Target blueprint to edit
	 * @param Properties Map of property name to new value
	 * @param OutChanges Returns list of changes made
	 * @return True if all edits successful
	 */
	static bool EditBlueprintProperties(UBlueprint* Blueprint, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutChanges);

	/**
	 * Set property value on a blueprint's CDO
	 * @param CDO Class default object
	 * @param Property Property to set
	 * @param Value New value as string
	 * @return True if successful
	 */
	static bool SetPropertyValue(UObject* CDO, FProperty* Property, const FString& Value);

	/**
	 * Get property value from CDO as string
	 * @param CDO Class default object
	 * @param Property Property to read
	 * @return Property value as string
	 */
	static FString GetPropertyValue(const UObject* CDO, const FProperty* Property);

	/**
	 * Compile a blueprint after modification
	 * @param Blueprint Blueprint to compile
	 * @return True if compilation successful
	 */
	static bool CompileBlueprint(UBlueprint* Blueprint);

	/**
	 * Save a blueprint to disk
	 * @param Blueprint Blueprint to save
	 * @return True if save successful
	 */
	static bool SaveBlueprint(UBlueprint* Blueprint);

	/**
	 * Mark blueprint for garbage collection cleanup
	 * @param Blueprint Blueprint to cleanup
	 */
	static void CleanupAfterEdit(UBlueprint* Blueprint);

	// ========================================================================
	// Graph Editing Capabilities
	// ========================================================================

	/**
	 * Add a member variable to a blueprint
	 * @param Blueprint Target blueprint
	 * @param VarName Name of variable
	 * @param VarType Type string (e.g. "bool", "int", "float", "Actor", "String")
	 * @param OutError Error message if failed
	 * @return Name of created variable (may be different if sanitized) or empty on fail
	 */
	static FString AddBlueprintVariable(UBlueprint* Blueprint, const FString& VarName, const FString& VarType, FString& OutError);

	/**
	 * Remove a member variable
	 */
	static bool RemoveBlueprintVariable(UBlueprint* Blueprint, const FString& VarName);

	/**
	 * Add a new function graph
	 * @param Blueprint Target blueprint
	 * @param FuncName Function name
	 * @param OutError Error message
	 * @return Name of created function
	 */
	static FString AddBlueprintFunction(UBlueprint* Blueprint, const FString& FuncName, FString& OutError);

	/**
	 * Add a node to a graph
	 * @param Blueprint Target blueprint
	 * @param GraphName Name of graph (e.g. "EventGraph", "MyFunc")
	 * @param NodeClassName Class of node to spawn (e.g. "K2Node_CallFunction")
	 * @param NodePosX X Position
	 * @param NodePosY Y Position
	 * @param Properties Optional properties to configure the node
	 * @param OutError Error message
	 * @return Pointer to created node or nullptr
	 */
	static class UEdGraphNode* AddGraphNode(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeClassName, int32 NodePosX, int32 NodePosY, const TMap<FString, FString>& Properties, FString& OutError);

	/**
	 * Connect two pins
	 * @param Blueprint Target blueprint
	 * @param GraphName Name of graph
	 * @param NodeAName Name/Title of first node
	 * @param PinAName Name of pin on first node
	 * @param NodeBName Name/Title of second node
	 * @param PinBName Name of pin on second node
	 * @param OutError Error message
	 * @return True if connected
	 */
	static bool ConnectNodePins(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeAName, const FString& PinAName, const FString& NodeBName, const FString& PinBName, FString& OutError);

	/**
	 * Remove a node from a graph
	 * @param Blueprint Target blueprint
	 * @param GraphName Name of graph
	 * @param NodeName Name/Title/GUID of node to remove
	 * @param OutError Error message
	 * @return True if removed
	 */
	static bool RemoveGraphNode(UBlueprint* Blueprint, const FString& GraphName, const FString& NodeName, FString& OutError);

	// ========================================================================
	// Component Editing Capabilities
	// ========================================================================

	/**
	 * Add a component to the blueprint
	 * @param Blueprint Target blueprint
	 * @param ComponentClassName Class of component (e.g. "StaticMeshComponent")
	 * @param ComponentName Name for the new component
	 * @param ParentName Name of parent component (optional)
	 * @param OutError Error message
	 * @return Name of created component
	 */
	static FString AddComponent(UBlueprint* Blueprint, const FString& ComponentClassName, const FString& ComponentName, const FString& ParentName, FString& OutError);

	/**
	 * Remove a component from the blueprint
	 * @param Blueprint Target blueprint
	 * @param ComponentName Name of component variable to remove
	 * @param OutError Error message
	 * @return True if successful
	 */
	static bool RemoveComponent(UBlueprint* Blueprint, const FString& ComponentName, FString& OutError);

private:

	/**
	 * Parse variable type string into pin type
	 */
	static bool ParseVariableType(const FString& TypeStr, struct FEdGraphPinType& OutType);

	/**
	 * Find a graph by name
	 */
	static class UEdGraph* FindGraphByName(UBlueprint* Blueprint, const FString& GraphName);

	/**
	 * Find a node in a graph by Name (GUID string) or Title
	 */
	static class UEdGraphNode* FindNodeInGraph(UEdGraph* Graph, const FString& NodeIdentifier);


	/**
	 * Parse string value to property type
	 * @param Property Target property
	 * @param ValueString String representation
	 * @param OutValue Parsed value pointer
	 * @return True if parsing successful
	 */
	static bool ParseValueForProperty(FProperty* Property, const FString& ValueString, void* OutValue);

	/**
	 * Export property value to string
	 * @param Property Property to export
	 * @param ValuePtr Pointer to value data
	 * @return String representation
	 */
	static FString ExportPropertyValue(const FProperty* Property, const void* ValuePtr);
};
