// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UClass;
class UDataAsset;
struct FQueryOptions;
class FJsonObject;

/**
 * Handles creation of Data Assets
 */
class FDataAssetFactoryHandler
{
public:

	/**
	 * Create a new Data Asset
	 * @param DataAssetClass The class of the Data Asset to create (must inherit from UDataAsset)
	 * @param AssetName Name of the new asset
	 * @param PackagePath Path to create the asset in (e.g. /Game/Data)
	 * @return Created Data Asset or nullptr
	 */
	static UDataAsset* CreateDataAsset(UClass* DataAssetClass, const FString& AssetName, const FString& PackagePath);

	/**
	 * Find a Data Asset class by name or path
	 * @param ClassName Class name or path
	 * @return Found class or nullptr
	 */
	static UClass* FindDataAssetClass(const FString& ClassName);

	/**
	 * Save a data asset to disk
	 * @param DataAsset Data asset to save
	 * @return True if save successful
	 */
	static bool SaveDataAsset(UDataAsset* DataAsset);

	/**
	 * Find an existing Data Asset by name
	 * @param DataAssetName Name of the Data Asset to find
	 * @return Found Data Asset or nullptr
	 */
	static UDataAsset* FindDataAsset(const FString& DataAssetName);

	/**
	 * Get all Data Assets of a specific class
	 * @param DataAssetClass Class to filter by (nullptr for all Data Assets)
	 * @param Options Query options for depth control
	 * @return JSON object containing results
	 */
	static TSharedPtr<FJsonObject> GetAllDataAssets(UClass* DataAssetClass, const FQueryOptions& Options);

	/**
	 * Get detailed information about a specific Data Asset
	 * @param DataAssetName Name of the Data Asset
	 * @param Options Query options for depth control
	 * @return JSON object containing Data Asset info or nullptr if not found
	 */
	static TSharedPtr<FJsonObject> GetDataAssetInfo(const FString& DataAssetName, const FQueryOptions& Options);

	/**
	 * Edit properties of a Data Asset
	 * @param DataAssetName Name of the Data Asset to edit
	 * @param Properties Map of property names to new values
	 * @param OutChanges Array to store change information
	 * @return True if successful
	 */
	static bool EditDataAssetProperties(const FString& DataAssetName, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutChanges);

private:
	
	/**
	 * Generate full package path
	 */
	static FString GeneratePackagePath(const FString& PackagePath, const FString& AssetName);

	/**
	 * Validate and clean package path
	 */
	static FString ValidateAndCleanPath(const FString& PackagePath);

	/**
	 * Extract property information from Data Asset class
	 */
	static void ExtractDataAssetProperties(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& PropertiesArray, const struct FQueryOptions& Options);

	/**
	 * Extract default values from Data Asset class
	 */
	static void ExtractDataAssetDefaults(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& DefaultsArray, const struct FQueryOptions& Options);

	/**
	 * Extract current values from Data Asset instance
	 */
	static void ExtractDataAssetCurrentValues(UDataAsset* DataAsset, TArray<TSharedPtr<FJsonValue>>& CurrentValuesArray, const struct FQueryOptions& Options);

	/**
	 * Extract function information from Data Asset class
	 */
	static void ExtractDataAssetFunctions(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& FunctionsArray, const struct FQueryOptions& Options);
};

