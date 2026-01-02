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
	// Spawn manager updates are handled by track generator when pieces are created
}

void USpawnManager::Reset()
{
	// Reset spawn manager state
}

void USpawnManager::SpawnOnTrackPiece(ATrackPiece* TrackPiece)
{
	if (!TrackPiece || !GameMode)
	{
		return;
	}

	UWorld* World = GameMode->GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayManager* GameplayManager = GameMode->GetGameplayManager();
	if (!GameplayManager)
	{
		return;
	}

	// Get spawn points from track piece
	const TArray<FSpawnPoint>& SpawnPoints = TrackPiece->GetSpawnPoints();
	FVector TrackPieceLocation = TrackPiece->GetActorLocation();

	for (const FSpawnPoint& SpawnPoint : SpawnPoints)
	{
		FVector SpawnLocation;
		bool bUseComponent = false;

		// Try to find spawn position by component name if provided
		if (!SpawnPoint.SpawnPositionComponentName.IsEmpty())
		{
			if (USceneComponent* SpawnComp = TrackPiece->FindComponentByName(SpawnPoint.SpawnPositionComponentName))
			{
				SpawnLocation = SpawnComp->GetComponentLocation();
				bUseComponent = true;
			}
		}

		// Fallback to Lane/ForwardPosition if no component or not found
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

		// Use seeded random if available
		FRandomStream* RandomStream = nullptr;
		if (GameMode)
		{
			RandomStream = &GameMode->GetSeededRandomStream();
		}

		float RandomValue = RandomStream ? RandomStream->FRand() : FMath::FRand();
		bool bShouldSpawn = false;

		switch (SpawnPoint.SpawnType)
		{
		case ESpawnPointType::Coin:
			bShouldSpawn = (RandomValue <= GameplayManager->GetCoinSpawnProbability());
			break;
		case ESpawnPointType::PowerUp:
			bShouldSpawn = (RandomValue <= GameplayManager->GetPowerUpSpawnProbability());
			break;
		case ESpawnPointType::Obstacle:
			bShouldSpawn = (RandomValue <= GameplayManager->GetObstacleSpawnProbability());
			break;
		}

		if (bShouldSpawn)
		{
			UDataAsset* SelectedDef = nullptr;
			TSubclassOf<AActor> ClassToSpawn = nullptr;
			EPlayerClass CurrentClass = GameMode->GetSelectedClass();

			// Logic for weighted selection from definitions
			if (SpawnPoint.WeightedDefinitions.Num() > 0)
			{
				TArray<FWeightedDefinition> ValidDefs;
				float TotalWeight = 0.0f;

				for (const FWeightedDefinition& WD : SpawnPoint.WeightedDefinitions)
				{
					if (!WD.Definition) continue;

					bool bClassAllowed = true;
					
					// Perform class validation based on the DA type
					if (UObstacleDefinition* OD = Cast<UObstacleDefinition>(WD.Definition))
					{
						if (OD->AllowedClasses.Num() > 0 && !OD->AllowedClasses.Contains(CurrentClass)) bClassAllowed = false;
					}
					else if (UPowerUpDefinition* PD = Cast<UPowerUpDefinition>(WD.Definition))
					{
						if (PD->AllowedClasses.Num() > 0 && !PD->AllowedClasses.Contains(CurrentClass)) bClassAllowed = false;
					}
					else if (UCollectibleDefinition* CD = Cast<UCollectibleDefinition>(WD.Definition))
					{
						if (CD->AllowedClasses.Num() > 0 && !CD->AllowedClasses.Contains(CurrentClass)) bClassAllowed = false;
					}

					if (bClassAllowed)
					{
						ValidDefs.Add(WD);
						TotalWeight += WD.Weight;
					}
				}

				if (ValidDefs.Num() > 0 && TotalWeight > 0.0f)
				{
					float SelectionRV = RandomStream ? RandomStream->FRandRange(0.0f, TotalWeight) : FMath::FRandRange(0.0f, TotalWeight);
					float CurrentWeightSum = 0.0f;
					for (const FWeightedDefinition& WD : ValidDefs)
					{
						CurrentWeightSum += WD.Weight;
						if (SelectionRV <= CurrentWeightSum)
						{
							SelectedDef = WD.Definition;
							break;
						}
					}
				}
			}

			// Determine final class to spawn
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

					// Apply properties from definition if selected
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
						// PowerUps apply themselves via definition during collection, usually
					}
				}
			}
		}
	}
}

