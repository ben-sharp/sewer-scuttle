// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExporter.h"
#include "ContentRegistry.h"
#include "TrackPieceDefinition.h"
#include "ObstacleDefinition.h"
#include "PowerUpDefinition.h"
#include "CollectibleDefinition.h"
#include "PlayerClass.h"
#include "ObstacleSpawnComponent.h"
#include "PowerUpSpawnComponent.h"
#include "CollectibleSpawnComponent.h"
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
		PropsObj->SetBoolField(TEXT("is_first_piece"), Def->bIsFirstPiece);

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
					TSharedPtr<FJsonObject> SpawnConfigObj = nullptr;

					if (UObstacleSpawnComponent* ObsComp = Cast<UObstacleSpawnComponent>(Comp))
					{
						SpawnConfigObj = MakeShareable(new FJsonObject);
						SpawnConfigObj->SetStringField(TEXT("spawn_type"), TEXT("Obstacle"));
						
						TArray<TSharedPtr<FJsonValue>> WeightedArray;
						for (const FWeightedDefinition& WD : ObsComp->ObstacleDefinitions)
						{
							if (!WD.Definition) continue;
							TSharedPtr<FJsonObject> WObj = MakeShareable(new FJsonObject);
							WObj->SetStringField(TEXT("id"), WD.Definition->GetName());
							WObj->SetNumberField(TEXT("weight"), WD.Weight);
							WeightedArray.Add(MakeShareable(new FJsonValueObject(WObj)));
						}
						SpawnConfigObj->SetArrayField(TEXT("weighted_definitions"), WeightedArray);
						
						// Export first valid class as fallback for legacy backend fields
						FString FallbackClass = TEXT("");
						TArray<FString> AllowedClasses;
						
						for (const FWeightedDefinition& WD : ObsComp->ObstacleDefinitions)
						{
							if (UObstacleDefinition* OD = Cast<UObstacleDefinition>(WD.Definition))
							{
								if (OD->ObstacleClass) { FallbackClass = OD->ObstacleClass->GetName(); }
								for (EPlayerClass PC : OD->AllowedClasses) AllowedClasses.AddUnique(FPlayerClassData::PlayerClassToString(PC));
							}
						}
						SpawnConfigObj->SetStringField(TEXT("spawn_class"), FallbackClass);
						
						TArray<TSharedPtr<FJsonValue>> AllowedClassesJson;
						for (const FString& AC : AllowedClasses) AllowedClassesJson.Add(MakeShareable(new FJsonValueString(AC)));
						SpawnConfigObj->SetArrayField(TEXT("allowed_classes"), AllowedClassesJson);
						
						SpawnConfigObj->SetNumberField(TEXT("probability"), ObsComp->SpawnProbability);
					}
					else if (UPowerUpSpawnComponent* PUComp = Cast<UPowerUpSpawnComponent>(Comp))
					{
						SpawnConfigObj = MakeShareable(new FJsonObject);
						SpawnConfigObj->SetStringField(TEXT("spawn_type"), TEXT("PowerUp"));
						
						TArray<TSharedPtr<FJsonValue>> WeightedArray;
						for (const FWeightedDefinition& WD : PUComp->PowerUpDefinitions)
						{
							if (!WD.Definition) continue;
							TSharedPtr<FJsonObject> WObj = MakeShareable(new FJsonObject);
							WObj->SetStringField(TEXT("id"), WD.Definition->GetName());
							WObj->SetNumberField(TEXT("weight"), WD.Weight);
							WeightedArray.Add(MakeShareable(new FJsonValueObject(WObj)));
						}
						SpawnConfigObj->SetArrayField(TEXT("weighted_definitions"), WeightedArray);

						FString FallbackClass = TEXT("");
						TArray<FString> AllowedClasses;
						
						for (const FWeightedDefinition& WD : PUComp->PowerUpDefinitions)
						{
							if (UPowerUpDefinition* PD = Cast<UPowerUpDefinition>(WD.Definition))
							{
								if (PD->PowerUpClass) { FallbackClass = PD->PowerUpClass->GetName(); }
								for (EPlayerClass PC : PD->AllowedClasses) AllowedClasses.AddUnique(FPlayerClassData::PlayerClassToString(PC));
							}
						}
						SpawnConfigObj->SetStringField(TEXT("spawn_class"), FallbackClass);
						
						TArray<TSharedPtr<FJsonValue>> AllowedClassesJson;
						for (const FString& AC : AllowedClasses) AllowedClassesJson.Add(MakeShareable(new FJsonValueString(AC)));
						SpawnConfigObj->SetArrayField(TEXT("allowed_classes"), AllowedClassesJson);
						
						SpawnConfigObj->SetNumberField(TEXT("probability"), PUComp->SpawnProbability);
					}
					else if (UCollectibleSpawnComponent* ColComp = Cast<UCollectibleSpawnComponent>(Comp))
					{
						SpawnConfigObj = MakeShareable(new FJsonObject);
						SpawnConfigObj->SetStringField(TEXT("spawn_type"), TEXT("Coin"));
						
						TArray<TSharedPtr<FJsonValue>> WeightedArray;
						for (const FWeightedDefinition& WD : ColComp->CollectibleDefinitions)
						{
							if (!WD.Definition) continue;
							TSharedPtr<FJsonObject> WObj = MakeShareable(new FJsonObject);
							WObj->SetStringField(TEXT("id"), WD.Definition->GetName());
							WObj->SetNumberField(TEXT("weight"), WD.Weight);
							WeightedArray.Add(MakeShareable(new FJsonValueObject(WObj)));
						}
						SpawnConfigObj->SetArrayField(TEXT("weighted_definitions"), WeightedArray);

						FString FallbackClass = TEXT("");
						TArray<FString> AllowedClasses;
						
						for (const FWeightedDefinition& WD : ColComp->CollectibleDefinitions)
						{
							if (UCollectibleDefinition* CD = Cast<UCollectibleDefinition>(WD.Definition))
							{
								if (CD->CollectibleClass) { FallbackClass = CD->CollectibleClass->GetName(); }
								for (EPlayerClass PC : CD->AllowedClasses) AllowedClasses.AddUnique(FPlayerClassData::PlayerClassToString(PC));
							}
						}
						SpawnConfigObj->SetStringField(TEXT("spawn_class"), FallbackClass);
						
						TArray<TSharedPtr<FJsonValue>> AllowedClassesJson;
						for (const FString& AC : AllowedClasses) AllowedClassesJson.Add(MakeShareable(new FJsonValueString(AC)));
						SpawnConfigObj->SetArrayField(TEXT("allowed_classes"), AllowedClassesJson);
						
						SpawnConfigObj->SetNumberField(TEXT("probability"), ColComp->CollectibleDefinitions.Num() > 0 ? ColComp->SpawnProbability : 0.0f);
					}

					if (SpawnConfigObj.IsValid())
					{
						USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
						SpawnConfigObj->SetStringField(TEXT("component_name"), Comp->GetName());
						
						// Try to determine lane based on Y position (-200, 0, 200)
						float Y = SceneComp->GetRelativeLocation().Y;
						int32 Lane = 1; // Center
						if (Y < -100.0f) Lane = 0;
						else if (Y > 100.0f) Lane = 2;
						
						SpawnConfigObj->SetNumberField(TEXT("lane"), Lane);
						SpawnConfigObj->SetNumberField(TEXT("forward_position"), SceneComp->GetRelativeLocation().X);
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
		for (ETrackTier Tier : Def->DifficultyAvailability)
		{
			TiersArray.Add(MakeShareable(new FJsonValueString(TrackTierUtils::ToString(Tier))));
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

