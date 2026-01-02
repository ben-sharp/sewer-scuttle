// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpawnManager.h"
#include "EndlessRunnerGameMode.h"
#include "TrackPiece.h"
#include "ContentRegistry.h"
#include "ObstacleDefinition.h"
#include "PowerUpDefinition.h"
#include "CollectibleDefinition.h"
#include "RabbitCharacter.h"
#include "CollectibleCoin.h"
#include "PowerUp.h"
#include "Obstacle.h"
#include "GameplayManager.h"
#include "Engine/World.h"

USpawnManager::USpawnManager()
{
	GameMode = nullptr;
}

void USpawnManager::Initialize(AEndlessRunnerGameMode* InGameMode)
{
	GameMode = InGameMode;
}

void USpawnManager::Update(float DeltaTime)
{
}

void USpawnManager::Reset()
{
}

void USpawnManager::SpawnOnTrackPiece(ATrackPiece* TrackPiece)
{
	if (!TrackPiece || !GameMode) return;

	UWorld* World = GameMode->GetWorld();
	if (!World) return;

	UGameplayManager* GameplayManager = GameMode->GetGameplayManager();
	if (!GameplayManager) return;

	const TArray<FSpawnPoint>& SpawnPoints = TrackPiece->GetSpawnPoints();
	FVector TrackPieceLocation = TrackPiece->GetActorLocation();

	for (const FSpawnPoint& SpawnPoint : SpawnPoints)
	{
		FVector SpawnLocation;
		bool bUseComponent = false;

		if (!SpawnPoint.SpawnPositionComponentName.IsEmpty())
		{
			if (USceneComponent* SpawnComp = TrackPiece->FindComponentByName(SpawnPoint.SpawnPositionComponentName))
			{
				SpawnLocation = SpawnComp->GetComponentLocation();
				bUseComponent = true;
			}
		}

		if (!bUseComponent)
		{
			float LaneY = 0.0f;
			switch (SpawnPoint.Lane)
			{
			case 0: LaneY = ARabbitCharacter::LANE_LEFT_Y; break;
			case 1: LaneY = ARabbitCharacter::LANE_CENTER_Y; break;
			case 2: LaneY = ARabbitCharacter::LANE_RIGHT_Y; break;
			default: LaneY = ARabbitCharacter::LANE_CENTER_Y; break;
			}
			SpawnLocation = TrackPieceLocation + FVector(SpawnPoint.ForwardPosition, LaneY, 0.0f);
		}

		FRandomStream* RandomStream = &GameMode->GetSeededRandomStream();
		float RandomValue = RandomStream->FRand();
		bool bShouldSpawn = false;
		FString PrescribedId = TEXT("");

		// SEEDED RUN: Strictly obey server
		if (TrackPiece->HasPrescribedSpawns())
		{
			PrescribedId = TrackPiece->GetPrescribedSpawn(SpawnPoint.SpawnPositionComponentName);
			// Only spawn if server explicitly provided a Definition ID in the map
			bShouldSpawn = !PrescribedId.IsEmpty();
		}
		// ENDLESS MODE: Local deterministic roll
		else
		{
			bShouldSpawn = (RandomValue <= SpawnPoint.SpawnProbability);
		}

		if (bShouldSpawn)
		{
			UBaseContentDefinition* SelectedDef = nullptr;
			TSubclassOf<AActor> ClassToSpawn = nullptr;
			EPlayerClass CurrentClass = GameMode->GetSelectedClass();

            if (TrackPiece->HasPrescribedSpawns())
            {
                if (UContentRegistry* Registry = GameMode->GetContentRegistry())
                {
                    // Search all registries since the spawn point is polymorphic
					SelectedDef = Registry->FindObstacleById(PrescribedId);
					if (!SelectedDef) SelectedDef = Registry->FindPowerUpById(PrescribedId);
					if (!SelectedDef) SelectedDef = Registry->FindCollectibleById(PrescribedId);
                }
            }
			else if (SpawnPoint.WeightedDefinitions.Num() > 0)
			{
				TArray<FWeightedDefinition> ValidDefs;
				float TotalWeight = 0.0f;

				for (const FWeightedDefinition& WD : SpawnPoint.WeightedDefinitions)
				{
					if (!WD.Definition) continue;
					if (WD.Definition->AllowedClasses.Num() == 0 || WD.Definition->AllowedClasses.Contains(CurrentClass))
					{
						ValidDefs.Add(WD);
						TotalWeight += WD.Weight;
					}
				}

				if (ValidDefs.Num() > 0 && TotalWeight > 0.0f)
				{
					float SelectionRV = RandomStream->FRandRange(0.0f, TotalWeight);
					float CurrentWeightSum = 0.0f;
					for (const FWeightedDefinition& WD : ValidDefs)
					{
						CurrentWeightSum += WD.Weight;
						if (SelectionRV <= CurrentWeightSum) { SelectedDef = WD.Definition; break; }
					}
				}
			}

			if (SelectedDef)
			{
				if (UObstacleDefinition* OD = Cast<UObstacleDefinition>(SelectedDef)) ClassToSpawn = OD->ObstacleClass;
				else if (UPowerUpDefinition* PD = Cast<UPowerUpDefinition>(SelectedDef)) ClassToSpawn = PD->PowerUpClass;
				else if (UCollectibleDefinition* CD = Cast<UCollectibleDefinition>(SelectedDef)) ClassToSpawn = CD->CollectibleClass;
			}

			if (ClassToSpawn)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AActor* SpawnedActor = World->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedActor)
				{
					TrackPiece->RegisterSpawnedActor(SpawnedActor);
					if (SelectedDef)
					{
						if (AObstacle* Obstacle = Cast<AObstacle>(SpawnedActor))
						{
							UObstacleDefinition* OD = Cast<UObstacleDefinition>(SelectedDef);
							Obstacle->SetObstacleType(OD->ObstacleType);
							Obstacle->SetDamage(OD->Damage);
						}
						else if (ACollectibleCoin* Coin = Cast<ACollectibleCoin>(SpawnedActor))
						{
							UCollectibleDefinition* CD = Cast<UCollectibleDefinition>(SelectedDef);
							Coin->SetValue(CD->Value);
						}
					}
				}
			}
		}
	}
}
