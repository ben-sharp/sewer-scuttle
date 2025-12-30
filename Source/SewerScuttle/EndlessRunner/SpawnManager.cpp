// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpawnManager.h"
#include "EndlessRunnerGameMode.h"
#include "TrackPiece.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"
#include "PowerUp.h"
#include "PowerUpSpawnComponent.h"
#include "Obstacle.h"
#include "ObstacleSpawnComponent.h"
#include "CollectibleSpawnComponent.h"
#include "Engine/World.h"
#include "Containers/Set.h"

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

	// Prevent duplicate spawning on the same track piece
	// Check if we've already spawned on this piece
	static TSet<ATrackPiece*> SpawnedPieces;
	if (SpawnedPieces.Contains(TrackPiece))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Skipping duplicate spawn on track piece '%s'"), *TrackPiece->GetName());
		return;
	}
	SpawnedPieces.Add(TrackPiece);

	UWorld* World = GameMode->GetWorld();
	if (!World)
	{
		return;
	}

	// Automatically spawn from any CollectibleSpawnComponents in the Blueprint (no data asset config needed)
	TArray<UActorComponent*> CollectibleComponents;
	TrackPiece->GetComponents(UCollectibleSpawnComponent::StaticClass(), CollectibleComponents);
	
	for (UActorComponent* Comp : CollectibleComponents)
	{
		if (UCollectibleSpawnComponent* CollectibleSpawnComp = Cast<UCollectibleSpawnComponent>(Comp))
		{
			// Validate spawn component before use
			if (!CollectibleSpawnComp || CollectibleSpawnComp->GetCollectibleEntryCount() == 0)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: CollectibleSpawnComponent invalid, skipping"));
				continue;
			}
			
			// Check spawn chance using seeded RNG
			float RandomValue = 0.0f;
			if (GameMode)
			{
				RandomValue = GameMode->GetSeededRandomStream().FRand();
			}
			else
			{
				// Fallback to global random if GameMode not available
				RandomValue = FMath::RandRange(0.0f, 1.0f);
			}
			
			if (RandomValue > CollectibleSpawnComp->SpawnChance)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: CollectibleSpawnComponent '%s' failed spawn chance check (%.2f > %.2f)"), 
					*CollectibleSpawnComp->GetName(), RandomValue, CollectibleSpawnComp->SpawnChance);
				continue;
			}

			// Get current player class
			EPlayerClass CurrentClass = EPlayerClass::Vanilla;
			if (GameMode)
			{
				CurrentClass = GameMode->GetSelectedClass();
			}
			
			UE_LOG(LogTemp, Log, TEXT("SpawnManager: Found CollectibleSpawnComponent '%s' in track piece '%s', selecting collectible for class %d"), 
				*CollectibleSpawnComp->GetName(), *TrackPiece->GetName(), (int32)CurrentClass);
			
			// Select a valid collectible for this class
			TSubclassOf<AActor> SelectedCollectibleClass = CollectibleSpawnComp->SelectCollectible(CurrentClass);
			
			// Validate selected class
			if (!SelectedCollectibleClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid collectible class selected, skipping spawn"));
				continue;
			}
			
			// Validate spawn location
			FVector SpawnLocation = CollectibleSpawnComp->GetComponentLocation();
			if (!FMath::IsFinite(SpawnLocation.X) || !FMath::IsFinite(SpawnLocation.Y) || !FMath::IsFinite(SpawnLocation.Z))
			{
				UE_LOG(LogTemp, Error, TEXT("SpawnManager: Invalid spawn location, skipping"));
				continue;
			}
			
			if (SelectedCollectibleClass)
			{
				// Spawn the selected collectible at the component's location
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				AActor* SpawnedActor = World->SpawnActor<AActor>(SelectedCollectibleClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedActor)
				{
					SpawnedActor->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
					TrackPiece->RegisterSpawnedActor(SpawnedActor);
					UE_LOG(LogTemp, Log, TEXT("SpawnManager: Auto-spawned collectible '%s' from component '%s' at location (%.2f, %.2f, %.2f)"), 
						*SelectedCollectibleClass->GetName(), *CollectibleSpawnComp->GetName(), 
						SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("SpawnManager: Failed to auto-spawn collectible '%s' from component '%s'"), 
						*SelectedCollectibleClass->GetName(), *CollectibleSpawnComp->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid collectible found for class %d in component '%s' (has %d entries)"), 
					(int32)CurrentClass, *CollectibleSpawnComp->GetName(), CollectibleSpawnComp->GetCollectibleEntryCount());
			}
		}
	}

	// Automatically spawn from any PowerUpSpawnComponents in the Blueprint (no data asset config needed)
	TArray<UActorComponent*> PowerUpComponents;
	TrackPiece->GetComponents(UPowerUpSpawnComponent::StaticClass(), PowerUpComponents);
	
	for (UActorComponent* Comp : PowerUpComponents)
	{
		if (UPowerUpSpawnComponent* PowerUpSpawnComp = Cast<UPowerUpSpawnComponent>(Comp))
		{
			// Validate spawn component before use
			if (!PowerUpSpawnComp || PowerUpSpawnComp->GetPowerUpEntryCount() == 0)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: PowerUpSpawnComponent invalid, skipping"));
				continue;
			}
			
			// Check spawn chance using seeded RNG
			float RandomValue = 0.0f;
			if (GameMode)
			{
				RandomValue = GameMode->GetSeededRandomStream().FRand();
			}
			else
			{
				// Fallback to global random if GameMode not available
				RandomValue = FMath::RandRange(0.0f, 1.0f);
			}
			
			if (RandomValue > PowerUpSpawnComp->SpawnChance)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: PowerUpSpawnComponent '%s' failed spawn chance check (%.2f > %.2f)"), 
					*PowerUpSpawnComp->GetName(), RandomValue, PowerUpSpawnComp->SpawnChance);
				continue;
			}

			// Get current player class
			EPlayerClass CurrentClass = EPlayerClass::Vanilla;
			if (GameMode)
			{
				CurrentClass = GameMode->GetSelectedClass();
			}
			
			UE_LOG(LogTemp, Log, TEXT("SpawnManager: Found PowerUpSpawnComponent '%s' in track piece '%s', selecting power-up for class %d"), 
				*PowerUpSpawnComp->GetName(), *TrackPiece->GetName(), (int32)CurrentClass);
			
			// Select a valid power-up for this class
			TSubclassOf<APowerUp> SelectedPowerUpClass = PowerUpSpawnComp->SelectPowerUp(CurrentClass);
			
			// Validate selected class
			if (!SelectedPowerUpClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid power-up class selected, skipping spawn"));
				continue;
			}
			
			// Validate spawn location
			FVector SpawnLocation = PowerUpSpawnComp->GetComponentLocation();
			if (!FMath::IsFinite(SpawnLocation.X) || !FMath::IsFinite(SpawnLocation.Y) || !FMath::IsFinite(SpawnLocation.Z))
			{
				UE_LOG(LogTemp, Error, TEXT("SpawnManager: Invalid spawn location, skipping"));
				continue;
			}
			
			if (SelectedPowerUpClass)
			{
				// Spawn the selected power-up at the component's location
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				AActor* SpawnedActor = World->SpawnActor<AActor>(SelectedPowerUpClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedActor)
				{
					SpawnedActor->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
					TrackPiece->RegisterSpawnedActor(SpawnedActor);
					UE_LOG(LogTemp, Log, TEXT("SpawnManager: Auto-spawned power-up '%s' from component '%s' at location (%.2f, %.2f, %.2f)"), 
						*SelectedPowerUpClass->GetName(), *PowerUpSpawnComp->GetName(), 
						SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("SpawnManager: Failed to auto-spawn power-up '%s' from component '%s'"), 
						*SelectedPowerUpClass->GetName(), *PowerUpSpawnComp->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid power-up found for class %d in component '%s' (has %d entries)"), 
					(int32)CurrentClass, *PowerUpSpawnComp->GetName(), PowerUpSpawnComp->GetPowerUpEntryCount());
			}
		}
	}

	// Automatically spawn from any ObstacleSpawnComponents in the Blueprint (no data asset config needed)
	TArray<UActorComponent*> ObstacleComponents;
	TrackPiece->GetComponents(UObstacleSpawnComponent::StaticClass(), ObstacleComponents);
	
	for (UActorComponent* Comp : ObstacleComponents)
	{
		if (UObstacleSpawnComponent* ObstacleSpawnComp = Cast<UObstacleSpawnComponent>(Comp))
		{
			// Validate spawn component before use
			if (!ObstacleSpawnComp || ObstacleSpawnComp->GetObstacleEntryCount() == 0)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: ObstacleSpawnComponent invalid, skipping"));
				continue;
			}
			
			// Check spawn chance using seeded RNG
			float RandomValue = 0.0f;
			if (GameMode)
			{
				RandomValue = GameMode->GetSeededRandomStream().FRand();
			}
			else
			{
				// Fallback to global random if GameMode not available
				RandomValue = FMath::RandRange(0.0f, 1.0f);
			}
			
			if (RandomValue > ObstacleSpawnComp->SpawnChance)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("SpawnManager: ObstacleSpawnComponent '%s' failed spawn chance check (%.2f > %.2f)"), 
					*ObstacleSpawnComp->GetName(), RandomValue, ObstacleSpawnComp->SpawnChance);
				continue;
			}

			// Get current player class
			EPlayerClass CurrentClass = EPlayerClass::Vanilla;
			if (GameMode)
			{
				CurrentClass = GameMode->GetSelectedClass();
			}
			
			UE_LOG(LogTemp, Log, TEXT("SpawnManager: Found ObstacleSpawnComponent '%s' in track piece '%s', selecting obstacle for class %d"), 
				*ObstacleSpawnComp->GetName(), *TrackPiece->GetName(), (int32)CurrentClass);
			
			// Select a valid obstacle for this class
			TSubclassOf<AObstacle> SelectedObstacleClass = ObstacleSpawnComp->SelectObstacle(CurrentClass);
			
			// Validate selected class
			if (!SelectedObstacleClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid obstacle class selected, skipping spawn"));
				continue;
			}
			
			// Validate spawn location
			FVector SpawnLocation = ObstacleSpawnComp->GetComponentLocation();
			if (!FMath::IsFinite(SpawnLocation.X) || !FMath::IsFinite(SpawnLocation.Y) || !FMath::IsFinite(SpawnLocation.Z))
			{
				UE_LOG(LogTemp, Error, TEXT("SpawnManager: Invalid spawn location, skipping"));
				continue;
			}
			
			if (SelectedObstacleClass)
			{
				// Spawn the selected obstacle at the component's location
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				AActor* SpawnedActor = World->SpawnActor<AActor>(SelectedObstacleClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (SpawnedActor)
				{
					SpawnedActor->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
					TrackPiece->RegisterSpawnedActor(SpawnedActor);
					UE_LOG(LogTemp, Log, TEXT("SpawnManager: Auto-spawned obstacle '%s' from component '%s' at location (%.2f, %.2f, %.2f)"), 
						*SelectedObstacleClass->GetName(), *ObstacleSpawnComp->GetName(), 
						SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("SpawnManager: Failed to auto-spawn obstacle '%s' from component '%s'"), 
						*SelectedObstacleClass->GetName(), *ObstacleSpawnComp->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid obstacle found for class %d in component '%s' (has %d entries)"), 
					(int32)CurrentClass, *ObstacleSpawnComp->GetName(), ObstacleSpawnComp->GetObstacleEntryCount());
			}
		}
	}

	// All spawning is now handled via auto-detected spawn components above
}

