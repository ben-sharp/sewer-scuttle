// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetQueryHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

TSharedPtr<FJsonObject> FAssetQueryHandler::QueryAssets(const FString& AssetType)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AssetsArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;

	if (AssetType.IsEmpty())
	{
		// Get all assets if no type specified
		AssetRegistry.GetAllAssets(AssetDataList, true);
	}
	else
	{
		// Try to find the class
		UClass* TargetClass = FindObject<UClass>(nullptr, *AssetType);
		if (TargetClass)
		{
			AssetRegistry.GetAssetsByClass(TargetClass->GetClassPathName(), AssetDataList, true);
		}
		else
		{
			// Try as a string path
			FTopLevelAssetPath ClassPath(AssetType);
			AssetRegistry.GetAssetsByClass(ClassPath, AssetDataList, true);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d assets of type '%s'"), AssetDataList.Num(), *AssetType);

	for (const FAssetData& AssetData : AssetDataList)
	{
		TSharedPtr<FJsonObject> AssetJson = ExtractAssetInfo(AssetData);
		AssetsArray.Add(MakeShared<FJsonValueObject>(AssetJson));
	}

	Result->SetArrayField(TEXT("assets"), AssetsArray);
	Result->SetNumberField(TEXT("count"), AssetsArray.Num());
	Result->SetStringField(TEXT("type_filter"), AssetType);
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FAssetQueryHandler::SearchAssets(const FString& SearchQuery)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AssetsArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAllAssets(AssetDataList, true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Searching %d assets for '%s'"), AssetDataList.Num(), *SearchQuery);

	// Filter assets by search query
	for (const FAssetData& AssetData : AssetDataList)
	{
		// Check if asset name or path contains the search query
		if (AssetData.AssetName.ToString().Contains(SearchQuery, ESearchCase::IgnoreCase) ||
			AssetData.GetObjectPathString().Contains(SearchQuery, ESearchCase::IgnoreCase) ||
			AssetData.AssetClassPath.ToString().Contains(SearchQuery, ESearchCase::IgnoreCase))
		{
			TSharedPtr<FJsonObject> AssetJson = ExtractAssetInfo(AssetData);
			AssetsArray.Add(MakeShared<FJsonValueObject>(AssetJson));
		}
	}

	Result->SetArrayField(TEXT("assets"), AssetsArray);
	Result->SetNumberField(TEXT("count"), AssetsArray.Num());
	Result->SetStringField(TEXT("query"), SearchQuery);
	Result->SetBoolField(TEXT("success"), true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d matching assets"), AssetsArray.Num());

	return Result;
}

TSharedPtr<FJsonObject> FAssetQueryHandler::ExtractAssetInfo(const FAssetData& AssetData)
{
	TSharedPtr<FJsonObject> AssetJson = MakeShared<FJsonObject>();
	
	AssetJson->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
	AssetJson->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
	AssetJson->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());
	AssetJson->SetStringField(TEXT("package"), AssetData.PackageName.ToString());

	return AssetJson;
}

