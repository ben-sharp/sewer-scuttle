// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentRegistry.h"
#include "TrackPieceDefinition.h"
#include "ObstacleDefinition.h"
#include "PowerUpDefinition.h"
#include "CollectibleDefinition.h"
#include "ShopItemDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void UContentRegistry::GatherContent()
{
	TrackPieces.Empty();
	Obstacles.Empty();
	PowerUps.Empty();
	Collectibles.Empty();
	ShopItems.Empty();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDataList;
	
	// Gather Track Pieces
	AssetRegistry.GetAssetsByClass(UTrackPieceDefinition::StaticClass()->GetClassPathName(), AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UTrackPieceDefinition* Def = Cast<UTrackPieceDefinition>(AssetData.GetAsset()))
		{
			TrackPieces.Add(Def);
		}
	}
	AssetDataList.Empty();

	// Gather Obstacles
	AssetRegistry.GetAssetsByClass(UObstacleDefinition::StaticClass()->GetClassPathName(), AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UObstacleDefinition* Def = Cast<UObstacleDefinition>(AssetData.GetAsset()))
		{
			Obstacles.Add(Def);
		}
	}
	AssetDataList.Empty();

	// Gather Power-ups
	AssetRegistry.GetAssetsByClass(UPowerUpDefinition::StaticClass()->GetClassPathName(), AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UPowerUpDefinition* Def = Cast<UPowerUpDefinition>(AssetData.GetAsset()))
		{
			PowerUps.Add(Def);
		}
	}
	AssetDataList.Empty();

	// Gather Collectibles
	AssetRegistry.GetAssetsByClass(UCollectibleDefinition::StaticClass()->GetClassPathName(), AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UCollectibleDefinition* Def = Cast<UCollectibleDefinition>(AssetData.GetAsset()))
		{
			Collectibles.Add(Def);
		}
	}
	AssetDataList.Empty();

	// Gather Shop Items
	AssetDataList.Empty();
	AssetRegistry.GetAssetsByClass(UShopItemDefinition::StaticClass()->GetClassPathName(), AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UShopItemDefinition* Def = Cast<UShopItemDefinition>(AssetData.GetAsset()))
		{
			ShopItems.Add(Def);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ContentRegistry: Gathered %d track pieces, %d obstacles, %d power-ups, %d collectibles, %d shop items"),
		TrackPieces.Num(), Obstacles.Num(), PowerUps.Num(), Collectibles.Num(), ShopItems.Num());
}

UTrackPieceDefinition* UContentRegistry::FindTrackPieceById(const FString& ContentId) const
{
	for (UTrackPieceDefinition* Def : TrackPieces)
	{
		if (Def && (Def->GetName() == ContentId || Def->PieceName == ContentId)) return Def;
	}
	return nullptr;
}

UObstacleDefinition* UContentRegistry::FindObstacleById(const FString& ContentId) const
{
	for (UObstacleDefinition* Def : Obstacles)
	{
		if (Def && (Def->GetName() == ContentId || Def->ObstacleName == ContentId)) return Def;
	}
	return nullptr;
}

UPowerUpDefinition* UContentRegistry::FindPowerUpById(const FString& ContentId) const
{
	for (UPowerUpDefinition* Def : PowerUps)
	{
		if (Def && (Def->GetName() == ContentId || Def->PowerUpName == ContentId)) return Def;
	}
	return nullptr;
}

UCollectibleDefinition* UContentRegistry::FindCollectibleById(const FString& ContentId) const
{
	for (UCollectibleDefinition* Def : Collectibles)
	{
		if (Def && (Def->GetName() == ContentId || Def->CollectibleName == ContentId)) return Def;
	}
	return nullptr;
}

UShopItemDefinition* UContentRegistry::FindShopItemById(const FString& ContentId) const
{
	for (UShopItemDefinition* Def : ShopItems)
	{
		if (Def && (Def->GetName() == ContentId || Def->ItemName == ContentId)) return Def;
	}
	return nullptr;
}

