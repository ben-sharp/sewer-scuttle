// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

/**
 * Handles general asset querying using the asset registry
 */
class FAssetQueryHandler
{
public:

	/** Query assets by type */
	static TSharedPtr<FJsonObject> QueryAssets(const FString& AssetType);

	/** Search assets by query string */
	static TSharedPtr<FJsonObject> SearchAssets(const FString& SearchQuery);

private:

	/** Extract basic asset information */
	static TSharedPtr<FJsonObject> ExtractAssetInfo(const struct FAssetData& AssetData);
};

