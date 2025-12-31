// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentRegistry.h"
#include "TrackPieceDefinition.h"
#include "ObstacleDefinition.h"
#include "PowerUpDefinition.h"
#include "CollectibleDefinition.h"
#include "Engine/DataAsset.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/UObjectIterator.h"

UContentRegistry::UContentRegistry()
{
	ContentVersion = TEXT("1.0.0");
}

void UContentRegistry::CollectAllContent()
{
	AllContent.Empty();
	CollectTrackPieces();
	CollectObstacles();
	CollectPowerUps();
	CollectCollectibles();
}

void UContentRegistry::CollectTrackPieces()
{
	// Find all TrackPieceDefinition assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UTrackPieceDefinition::StaticClass()->GetClassPathName(), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UTrackPieceDefinition* Definition = Cast<UTrackPieceDefinition>(AssetData.GetAsset());
		if (Definition)
		{
			FContentDefinition ContentDef = ConvertTrackPiece(Definition);
			AllContent.Add(ContentDef);
		}
	}
}

void UContentRegistry::CollectObstacles()
{
	// Find all ObstacleDefinition data assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UObstacleDefinition::StaticClass()->GetClassPathName(), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UObstacleDefinition* Definition = Cast<UObstacleDefinition>(AssetData.GetAsset());
		if (Definition)
		{
			FContentDefinition ContentDef = ConvertObstacle(Definition);
			AllContent.Add(ContentDef);
		}
	}
}

void UContentRegistry::CollectPowerUps()
{
	// Find all PowerUpDefinition data assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UPowerUpDefinition::StaticClass()->GetClassPathName(), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UPowerUpDefinition* Definition = Cast<UPowerUpDefinition>(AssetData.GetAsset());
		if (Definition)
		{
			FContentDefinition ContentDef = ConvertPowerUp(Definition);
			AllContent.Add(ContentDef);
		}
	}
}

void UContentRegistry::CollectCollectibles()
{
	// Find all CollectibleDefinition data assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByClass(UCollectibleDefinition::StaticClass()->GetClassPathName(), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UCollectibleDefinition* Definition = Cast<UCollectibleDefinition>(AssetData.GetAsset());
		if (Definition)
		{
			FContentDefinition ContentDef = ConvertCollectible(Definition);
			AllContent.Add(ContentDef);
		}
	}
}

FContentDefinition UContentRegistry::ConvertTrackPiece(UTrackPieceDefinition* Definition) const
{
	FContentDefinition ContentDef;
	ContentDef.Type = TEXT("track_piece");
	ContentDef.Id = Definition->GetName();
	ContentDef.Name = Definition->PieceName;

	// Convert properties to key-value pairs
	ContentDef.Properties.Add(TEXT("length"), FString::SanitizeFloat(Definition->Length));
	ContentDef.Properties.Add(TEXT("min_difficulty"), FString::FromInt(Definition->MinDifficulty));
	ContentDef.Properties.Add(TEXT("max_difficulty"), Definition->MaxDifficulty == -1 ? TEXT("-1") : FString::FromInt(Definition->MaxDifficulty));
	ContentDef.Properties.Add(TEXT("weight"), FString::FromInt(Definition->SelectionWeight));
	ContentDef.Properties.Add(TEXT("is_first_piece"), Definition->bIsFirstPiece ? TEXT("true") : TEXT("false"));

	// Convert spawn configs to JSON-like string (simplified)
	FString SpawnConfigsStr = TEXT("[");
	for (int32 i = 0; i < Definition->SpawnConfigs.Num(); i++)
	{
		const FTrackPieceSpawnConfig& Config = Definition->SpawnConfigs[i];
		if (i > 0) SpawnConfigsStr += TEXT(",");
		SpawnConfigsStr += FString::Printf(TEXT("{\"lane\":%d,\"forward_position\":%.2f,\"spawn_type\":\"%s\",\"spawn_probability\":%.2f}"),
			Config.Lane,
			Config.ForwardPosition,
			Config.SpawnType == ESpawnPointType::Coin ? TEXT("coin") : (Config.SpawnType == ESpawnPointType::Obstacle ? TEXT("obstacle") : TEXT("powerup")),
			Config.SpawnProbability);
	}
	SpawnConfigsStr += TEXT("]");
	ContentDef.Properties.Add(TEXT("spawn_configs"), SpawnConfigsStr);

	return ContentDef;
}

FContentDefinition UContentRegistry::ConvertObstacle(UObstacleDefinition* Definition) const
{
	FContentDefinition ContentDef;
	ContentDef.Type = TEXT("obstacle");
	ContentDef.Id = Definition->GetName();
	ContentDef.Name = Definition->ObstacleName;

	// Convert obstacle properties
	ContentDef.Properties.Add(TEXT("obstacle_type"), 
		Definition->ObstacleType == EObstacleType::Low ? TEXT("low") : 
		(Definition->ObstacleType == EObstacleType::High ? TEXT("high") : TEXT("full")));
	ContentDef.Properties.Add(TEXT("damage"), FString::SanitizeFloat(Definition->Damage));
	ContentDef.Properties.Add(TEXT("lives_lost"), FString::FromInt(Definition->LivesLost));
	ContentDef.Properties.Add(TEXT("is_breakable"), Definition->bIsBreakable ? TEXT("true") : TEXT("false"));
	ContentDef.Properties.Add(TEXT("ignore_last_stand"), Definition->bIgnoreLastStand ? TEXT("true") : TEXT("false"));
	
	// Convert allowed classes array
	FString AllowedClassesStr = TEXT("[");
	for (int32 i = 0; i < Definition->AllowedClasses.Num(); i++)
	{
		if (i > 0) AllowedClassesStr += TEXT(",");
		FString ClassName;
		switch (Definition->AllowedClasses[i])
		{
			case EPlayerClass::Vanilla: ClassName = TEXT("vanilla"); break;
			case EPlayerClass::Rogue: ClassName = TEXT("rogue"); break;
			case EPlayerClass::Enforcer: ClassName = TEXT("enforcer"); break;
			case EPlayerClass::Joker: ClassName = TEXT("joker"); break;
			case EPlayerClass::Scout: ClassName = TEXT("scout"); break;
			case EPlayerClass::Collector: ClassName = TEXT("collector"); break;
			default: ClassName = TEXT("unknown"); break;
		}
		AllowedClassesStr += FString::Printf(TEXT("\"%s\""), *ClassName);
	}
	AllowedClassesStr += TEXT("]");
	ContentDef.Properties.Add(TEXT("allowed_classes"), AllowedClassesStr);
	
	ContentDef.Properties.Add(TEXT("weight"), FString::FromInt(Definition->SelectionWeight));

	return ContentDef;
}

FContentDefinition UContentRegistry::ConvertPowerUp(UPowerUpDefinition* Definition) const
{
	FContentDefinition ContentDef;
	ContentDef.Type = TEXT("powerup");
	ContentDef.Id = Definition->GetName();
	ContentDef.Name = Definition->PowerUpName;

	// Convert powerup properties
	ContentDef.Properties.Add(TEXT("duration"), FString::SanitizeFloat(Definition->Duration));
	
	// Convert allowed classes array
	FString AllowedClassesStr = TEXT("[");
	for (int32 i = 0; i < Definition->AllowedClasses.Num(); i++)
	{
		if (i > 0) AllowedClassesStr += TEXT(",");
		FString ClassName;
		switch (Definition->AllowedClasses[i])
		{
			case EPlayerClass::Vanilla: ClassName = TEXT("vanilla"); break;
			case EPlayerClass::Rogue: ClassName = TEXT("rogue"); break;
			case EPlayerClass::Enforcer: ClassName = TEXT("enforcer"); break;
			case EPlayerClass::Joker: ClassName = TEXT("joker"); break;
			case EPlayerClass::Scout: ClassName = TEXT("scout"); break;
			case EPlayerClass::Collector: ClassName = TEXT("collector"); break;
			default: ClassName = TEXT("unknown"); break;
		}
		AllowedClassesStr += FString::Printf(TEXT("\"%s\""), *ClassName);
	}
	AllowedClassesStr += TEXT("]");
	ContentDef.Properties.Add(TEXT("allowed_classes"), AllowedClassesStr);

	// GAS properties
	if (Definition->PermanentStatModifierEffect)
	{
		ContentDef.Properties.Add(TEXT("permanent_stat_modifier_effect"), Definition->PermanentStatModifierEffect->GetName());
	}
	if (Definition->TemporaryMultiplierEffect)
	{
		ContentDef.Properties.Add(TEXT("temporary_multiplier_effect"), Definition->TemporaryMultiplierEffect->GetName());
	}
	if (!Definition->StatTypeToModify.IsNone())
	{
		ContentDef.Properties.Add(TEXT("stat_type_to_modify"), Definition->StatTypeToModify.ToString());
	}
	ContentDef.Properties.Add(TEXT("modification_value"), FString::SanitizeFloat(Definition->ModificationValue));
	ContentDef.Properties.Add(TEXT("weight"), FString::FromInt(Definition->SelectionWeight));

	return ContentDef;
}

FContentDefinition UContentRegistry::ConvertCollectible(UCollectibleDefinition* Definition) const
{
	FContentDefinition ContentDef;
	ContentDef.Type = TEXT("collectible");
	ContentDef.Id = Definition->GetName();
	ContentDef.Name = Definition->CollectibleName;

	// Convert collectible properties
	ContentDef.Properties.Add(TEXT("value"), FString::FromInt(Definition->Value));
	ContentDef.Properties.Add(TEXT("default_item_value"), FString::FromInt(Definition->DefaultItemValue));
	ContentDef.Properties.Add(TEXT("is_magnetable"), Definition->bMagnetable ? TEXT("true") : TEXT("false"));
	ContentDef.Properties.Add(TEXT("is_special"), Definition->bIsSpecial ? TEXT("true") : TEXT("false"));
	ContentDef.Properties.Add(TEXT("special_value_multiplier"), FString::SanitizeFloat(Definition->SpecialValueMultiplier));
	
	// Convert allowed classes array
	FString AllowedClassesStr = TEXT("[");
	for (int32 i = 0; i < Definition->AllowedClasses.Num(); i++)
	{
		if (i > 0) AllowedClassesStr += TEXT(",");
		FString ClassName;
		switch (Definition->AllowedClasses[i])
		{
			case EPlayerClass::Vanilla: ClassName = TEXT("vanilla"); break;
			case EPlayerClass::Rogue: ClassName = TEXT("rogue"); break;
			case EPlayerClass::Enforcer: ClassName = TEXT("enforcer"); break;
			case EPlayerClass::Joker: ClassName = TEXT("joker"); break;
			case EPlayerClass::Scout: ClassName = TEXT("scout"); break;
			case EPlayerClass::Collector: ClassName = TEXT("collector"); break;
			default: ClassName = TEXT("unknown"); break;
		}
		AllowedClassesStr += FString::Printf(TEXT("\"%s\""), *ClassName);
	}
	AllowedClassesStr += TEXT("]");
	ContentDef.Properties.Add(TEXT("allowed_classes"), AllowedClassesStr);
	
	ContentDef.Properties.Add(TEXT("weight"), FString::FromInt(Definition->SelectionWeight));

	return ContentDef;
}

TArray<FContentDefinition> UContentRegistry::GetContentByType(const FString& Type) const
{
	TArray<FContentDefinition> Filtered;
	for (const FContentDefinition& Def : AllContent)
	{
		if (Def.Type == Type)
		{
			Filtered.Add(Def);
		}
	}
	return Filtered;
}

