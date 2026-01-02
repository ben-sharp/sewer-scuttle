// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExporter.h"
#include "ContentRegistry.h"
#include "TrackPieceDefinition.h"
#include "ObstacleDefinition.h"
#include "PowerUpDefinition.h"
#include "CollectibleDefinition.h"
#include "ShopItemDefinition.h"
#include "PlayerClass.h"
#include "Obstacle.h"
#include "PowerUp.h"
#include "CollectibleCoin.h"
#include "TrackPiece.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

FString UContentExporter::ExportToJson(UContentRegistry* Registry, const FString& Version)
{
	if (!Registry) return TEXT("");

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
	RootObject->SetStringField(TEXT("version"), Version);

	TArray<TSharedPtr<FJsonValue>> DefinitionsArray;

	// Export Track Pieces
	for (UTrackPieceDefinition* Def : Registry->GetTrackPieces())
	{
		TSharedPtr<FJsonObject> DefObj = MakeShareable(new FJsonObject);
		DefObj->SetStringField(TEXT("content_id"), Def->GetName());
		DefObj->SetStringField(TEXT("name"), Def->PieceName);
		DefObj->SetStringField(TEXT("type"), TEXT("track_piece"));

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject);
		PropsObj->SetNumberField(TEXT("length"), Def->Length);
		PropsObj->SetNumberField(TEXT("lane_width"), Def->LaneWidth);
		PropsObj->SetStringField(TEXT("piece_type"), TrackPieceUtils::ToString(Def->PieceType));

		// Export Spawn Configs
		TArray<TSharedPtr<FJsonValue>> SpawnConfigsArray;
		
		// Export from Blueprint components (if using BP actor)
		if (Def->bUseBlueprintActor && Def->BlueprintActorClass)
		{
			if (ATrackPiece* CDO = Cast<ATrackPiece>(Def->BlueprintActorClass->GetDefaultObject()))
			{
				TArray<UActorComponent*> Components;
				CDO->GetComponents(USceneComponent::StaticClass(), Components);

				for (UActorComponent* Comp : Components)
				{
					if (USmartSpawnComponent* SmartComp = Cast<USmartSpawnComponent>(Comp))
					{
						TSharedPtr<FJsonObject> SpawnConfigObj = MakeShareable(new FJsonObject);
						SpawnConfigObj->SetStringField(TEXT("component_name"), Comp->GetName());
						SpawnConfigObj->SetStringField(TEXT("spawn_type"), TEXT("Mixed")); // Now polymorphic
						SpawnConfigObj->SetNumberField(TEXT("probability"), SmartComp->SpawnProbability);

						TArray<TSharedPtr<FJsonValue>> WeightedArray;
						TArray<FString> CombinedAllowedClasses;
						FString FallbackClass = TEXT("");

						for (const FWeightedDefinition& WD : SmartComp->Definitions)
						{
							if (!WD.Definition) continue;

							TSharedPtr<FJsonObject> WObj = MakeShareable(new FJsonObject);
							WObj->SetStringField(TEXT("id"), WD.Definition->GetName());
							WObj->SetNumberField(TEXT("weight"), WD.Weight);
							WeightedArray.Add(MakeShareable(new FJsonValueObject(WObj)));

							// Collect allowed classes and fallback class from definitions
							for (EPlayerClass PC : WD.Definition->AllowedClasses)
							{
								CombinedAllowedClasses.AddUnique(FPlayerClassData::PlayerClassToString(PC));
							}

							if (FallbackClass.IsEmpty())
							{
								if (UObstacleDefinition* OD = Cast<UObstacleDefinition>(WD.Definition)) FallbackClass = OD->ObstacleClass ? OD->ObstacleClass->GetName() : TEXT("");
								else if (UPowerUpDefinition* PD = Cast<UPowerUpDefinition>(WD.Definition)) FallbackClass = PD->PowerUpClass ? PD->PowerUpClass->GetName() : TEXT("");
								else if (UCollectibleDefinition* CD = Cast<UCollectibleDefinition>(WD.Definition)) FallbackClass = CD->CollectibleClass ? CD->CollectibleClass->GetName() : TEXT("");
							}
						}

						SpawnConfigObj->SetArrayField(TEXT("weighted_definitions"), WeightedArray);
						SpawnConfigObj->SetStringField(TEXT("spawn_class"), FallbackClass);

						TArray<TSharedPtr<FJsonValue>> AllowedClassesJson;
						for (const FString& AC : CombinedAllowedClasses) AllowedClassesJson.Add(MakeShareable(new FJsonValueString(AC)));
						SpawnConfigObj->SetArrayField(TEXT("allowed_classes"), AllowedClassesJson);

						// Position
						float Y = SmartComp->GetRelativeLocation().Y;
						int32 Lane = (Y < -100.0f) ? 0 : ((Y > 100.0f) ? 2 : 1);
						SpawnConfigObj->SetNumberField(TEXT("lane"), Lane);
						SpawnConfigObj->SetNumberField(TEXT("forward_position"), SmartComp->GetRelativeLocation().X);

						SpawnConfigsArray.Add(MakeShareable(new FJsonValueObject(SpawnConfigObj)));
					}
				}
			}
		}
		
		PropsObj->SetArrayField(TEXT("spawn_configs"), SpawnConfigsArray);
		DefObj->SetObjectField(TEXT("properties"), PropsObj);
		DefinitionsArray.Add(MakeShareable(new FJsonValueObject(DefObj)));
	}

	// Export Obstacles
	for (UObstacleDefinition* Def : Registry->GetObstacles())
	{
		TSharedPtr<FJsonObject> DefObj = MakeShareable(new FJsonObject);
		DefObj->SetStringField(TEXT("content_id"), Def->GetName());
		DefObj->SetStringField(TEXT("name"), Def->ObstacleName);
		DefObj->SetStringField(TEXT("type"), TEXT("obstacle"));

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject);
		PropsObj->SetStringField(TEXT("obstacle_type"), ObstacleUtils::ToString(Def->ObstacleType));
		PropsObj->SetNumberField(TEXT("damage"), Def->Damage);
		PropsObj->SetNumberField(TEXT("lives_lost"), Def->LivesLost);
		
		TArray<TSharedPtr<FJsonValue>> ClassesArray;
		for (EPlayerClass PlayerClass : Def->AllowedClasses)
		{
			ClassesArray.Add(MakeShareable(new FJsonValueString(FPlayerClassData::PlayerClassToString(PlayerClass))));
		}
		PropsObj->SetArrayField(TEXT("allowed_classes"), ClassesArray);

		DefObj->SetObjectField(TEXT("properties"), PropsObj);
		DefinitionsArray.Add(MakeShareable(new FJsonValueObject(DefObj)));
	}

	// Export Power-ups
	for (UPowerUpDefinition* Def : Registry->GetPowerUps())
	{
		TSharedPtr<FJsonObject> DefObj = MakeShareable(new FJsonObject);
		DefObj->SetStringField(TEXT("content_id"), Def->GetName());
		DefObj->SetStringField(TEXT("name"), Def->PowerUpName);
		DefObj->SetStringField(TEXT("type"), TEXT("powerup"));

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject);
		PropsObj->SetNumberField(TEXT("duration"), Def->Duration);
		PropsObj->SetStringField(TEXT("stat_type"), Def->StatTypeToModify.ToString());
		PropsObj->SetNumberField(TEXT("modification_value"), Def->ModificationValue);
		PropsObj->SetNumberField(TEXT("base_cost"), (double)Def->BaseCost);
		PropsObj->SetNumberField(TEXT("cost_multiplier_per_tier"), (double)Def->CostMultiplierPerTier);
		
		TArray<TSharedPtr<FJsonValue>> TiersArray;
		if (Def->DifficultyAvailability.Num() > 0)
		{
			for (ETrackTier Tier : Def->DifficultyAvailability)
			{
				TiersArray.Add(MakeShareable(new FJsonValueString(TrackTierUtils::ToString(Tier))));
			}
		}
		else
		{
			// Fallback to all tiers if empty
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T1"))));
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T2"))));
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T3"))));
		}
		PropsObj->SetArrayField(TEXT("difficulty_availability"), TiersArray);

		TArray<TSharedPtr<FJsonValue>> ClassesArray;
		for (EPlayerClass PlayerClass : Def->AllowedClasses)
		{
			ClassesArray.Add(MakeShareable(new FJsonValueString(FPlayerClassData::PlayerClassToString(PlayerClass))));
		}
		PropsObj->SetArrayField(TEXT("allowed_classes"), ClassesArray);
		
		DefObj->SetObjectField(TEXT("properties"), PropsObj);
		DefinitionsArray.Add(MakeShareable(new FJsonValueObject(DefObj)));
	}

	// Export Collectibles
	for (UCollectibleDefinition* Def : Registry->GetCollectibles())
	{
		TSharedPtr<FJsonObject> DefObj = MakeShareable(new FJsonObject);
		DefObj->SetStringField(TEXT("content_id"), Def->GetName());
		DefObj->SetStringField(TEXT("name"), Def->CollectibleName);
		DefObj->SetStringField(TEXT("type"), TEXT("collectible"));

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject);
		PropsObj->SetNumberField(TEXT("value"), Def->Value);
		
		TArray<TSharedPtr<FJsonValue>> ClassesArray;
		for (EPlayerClass PlayerClass : Def->AllowedClasses)
		{
			ClassesArray.Add(MakeShareable(new FJsonValueString(FPlayerClassData::PlayerClassToString(PlayerClass))));
		}
		PropsObj->SetArrayField(TEXT("allowed_classes"), ClassesArray);

		DefObj->SetObjectField(TEXT("properties"), PropsObj);
		DefinitionsArray.Add(MakeShareable(new FJsonValueObject(DefObj)));
	}

	// Export Shop Items
	for (UShopItemDefinition* Def : Registry->GetShopItems())
	{
		TSharedPtr<FJsonObject> DefObj = MakeShareable(new FJsonObject);
		DefObj->SetStringField(TEXT("content_id"), Def->GetName());
		DefObj->SetStringField(TEXT("name"), Def->ItemName);
		DefObj->SetStringField(TEXT("type"), TEXT("shop_item"));

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject);
		PropsObj->SetNumberField(TEXT("base_cost"), Def->BaseCost);
		PropsObj->SetStringField(TEXT("item_type"), Def->ItemType == EShopItemType::PowerUp ? TEXT("PowerUp") : 
													Def->ItemType == EShopItemType::StatModifier ? TEXT("StatModifier") :
													Def->ItemType == EShopItemType::Collectible ? TEXT("Collectible") : TEXT("Custom"));
		PropsObj->SetStringField(TEXT("effect_tag"), Def->EffectTag.ToString());
		PropsObj->SetNumberField(TEXT("effect_value"), Def->EffectValue);
		PropsObj->SetStringField(TEXT("description"), Def->Description.ToString());
		PropsObj->SetBoolField(TEXT("can_be_boss_reward"), Def->bCanBeBossReward);

		TArray<TSharedPtr<FJsonValue>> ClassesArray;
		for (EPlayerClass PlayerClass : Def->AllowedClasses)
		{
			ClassesArray.Add(MakeShareable(new FJsonValueString(FPlayerClassData::PlayerClassToString(PlayerClass))));
		}
		PropsObj->SetArrayField(TEXT("allowed_classes"), ClassesArray);

		TArray<TSharedPtr<FJsonValue>> TiersArray;
		if (Def->DifficultyAvailability.Num() > 0)
		{
			for (ETrackTier Tier : Def->DifficultyAvailability)
			{
				TiersArray.Add(MakeShareable(new FJsonValueString(TrackTierUtils::ToString(Tier))));
			}
		}
		else
		{
			// Fallback to all tiers if empty
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T1"))));
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T2"))));
			TiersArray.Add(MakeShareable(new FJsonValueString(TEXT("T3"))));
		}
		PropsObj->SetArrayField(TEXT("difficulty_availability"), TiersArray);

		DefObj->SetObjectField(TEXT("properties"), PropsObj);
		DefinitionsArray.Add(MakeShareable(new FJsonValueObject(DefObj)));
	}

	RootObject->SetArrayField(TEXT("definitions"), DefinitionsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

bool UContentExporter::ExportToFile(UContentRegistry* Registry, const FString& Version, const FString& FilePath)
{
	FString JsonString = ExportToJson(Registry, Version);
	if (JsonString.IsEmpty()) return false;

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}
