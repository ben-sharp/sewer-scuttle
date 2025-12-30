// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayManager.h"
#include "EndlessRunnerGameMode.h"
#include "TrackGenerator.h"
#include "SpawnManager.h"

UGameplayManager::UGameplayManager()
{
	GameMode = nullptr;
	CurrentDifficulty = 0;
	CoinSpawnProbability = BaseCoinSpawnProbability;
	PowerUpSpawnProbability = BasePowerUpSpawnProbability;
	ObstacleSpawnProbability = BaseObstacleSpawnProbability;
}

void UGameplayManager::Initialize(AEndlessRunnerGameMode* InGameMode)
{
	GameMode = InGameMode;

	// Create spawn manager
	SpawnManager = NewObject<USpawnManager>(this);
	if (SpawnManager)
	{
		SpawnManager->Initialize(GameMode);
	}
}

void UGameplayManager::Update(float DeltaTime, float DistanceTraveled)
{
	// Update difficulty
	UpdateDifficulty(DistanceTraveled);

	// Update spawn probabilities
	UpdateSpawnProbabilities();

	// Update spawn manager
	if (SpawnManager)
	{
		SpawnManager->Update(DeltaTime);
	}
}

void UGameplayManager::Reset()
{
	CurrentDifficulty = 0;
	CoinSpawnProbability = BaseCoinSpawnProbability;
	PowerUpSpawnProbability = BasePowerUpSpawnProbability;
	ObstacleSpawnProbability = BaseObstacleSpawnProbability;

	if (SpawnManager)
	{
		SpawnManager->Reset();
	}
}

void UGameplayManager::UpdateDifficulty(float DistanceTraveled)
{
	int32 NewDifficulty = FMath::FloorToInt(DistanceTraveled / DifficultyIncreaseDistance);
	if (NewDifficulty > CurrentDifficulty)
	{
		CurrentDifficulty = NewDifficulty;

		// Update track generator difficulty
		if (GameMode && GameMode->GetTrackGenerator())
		{
			GameMode->GetTrackGenerator()->SetCurrentDifficulty(CurrentDifficulty);
		}
	}
}

void UGameplayManager::UpdateSpawnProbabilities()
{
	// Increase spawn probabilities with difficulty (capped)
	float DifficultyMultiplier = 1.0f + (CurrentDifficulty * 0.1f);

	CoinSpawnProbability = FMath::Min(BaseCoinSpawnProbability * DifficultyMultiplier, 0.8f);
	PowerUpSpawnProbability = FMath::Min(BasePowerUpSpawnProbability * DifficultyMultiplier, 0.3f);
	ObstacleSpawnProbability = FMath::Min(BaseObstacleSpawnProbability * DifficultyMultiplier, 0.6f);
}

