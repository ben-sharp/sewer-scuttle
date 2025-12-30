// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObstacleSpawnComponent.h"
#include "Obstacle.h"

UObstacleSpawnComponent::UObstacleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TSubclassOf<AObstacle> UObstacleSpawnComponent::SelectObstacle(EPlayerClass PlayerClass) const
{
	TArray<FObstacleSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid obstacles
	for (const FObstacleSpawnEntry& Entry : ObstacleEntries)
	{
		if (Entry.ObstacleClass)
		{
			// Check if obstacle allows this player class
			bool bIsValidForClass = true;
			if (Entry.AllowedClasses.Num() > 0)
			{
				bIsValidForClass = Entry.AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidEntries.Add(Entry);
				TotalWeight += Entry.Weight;
			}
		}
	}

	// If no valid entries and we don't require valid class, return first available
	if (ValidEntries.Num() == 0)
	{
		if (!bRequireValidClass && ObstacleEntries.Num() > 0 && ObstacleEntries[0].ObstacleClass)
		{
			return ObstacleEntries[0].ObstacleClass;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("ObstacleSpawnComponent: No valid obstacles found for class %d"), (int32)PlayerClass);
		return nullptr;
	}

	// Weighted random selection
	if (TotalWeight > 0.0f)
	{
		float RandomValue = FMath::RandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;

		for (const FObstacleSpawnEntry& Entry : ValidEntries)
		{
			CurrentWeight += Entry.Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.ObstacleClass;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].ObstacleClass;
}

TArray<TSubclassOf<AObstacle>> UObstacleSpawnComponent::GetValidObstacles(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<AObstacle>> ValidObstacles;

	for (const FObstacleSpawnEntry& Entry : ObstacleEntries)
	{
		if (Entry.ObstacleClass)
		{
			// Check if obstacle allows this player class
			bool bIsValidForClass = true;
			if (Entry.AllowedClasses.Num() > 0)
			{
				bIsValidForClass = Entry.AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidObstacles.Add(Entry.ObstacleClass);
			}
		}
	}

	return ValidObstacles;
}



