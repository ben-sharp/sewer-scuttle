// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObstacleSpawnComponent.h"
#include "Obstacle.h"
#include "ObstacleDefinition.h"

UObstacleSpawnComponent::UObstacleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UObstacleDefinition* UObstacleSpawnComponent::SelectObstacleDefinition(EPlayerClass PlayerClass) const
{
	TArray<FObstacleSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid obstacles
	for (const FObstacleSpawnEntry& Entry : ObstacleEntries)
	{
		if (Entry.ObstacleDefinition && Entry.ObstacleDefinition->ObstacleClass)
		{
			// Check if obstacle allows this player class (use override if set, otherwise check data asset)
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.ObstacleDefinition->AllowedClasses;
			
			if (AllowedClasses.Num() > 0)
			{
				bIsValidForClass = AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidEntries.Add(Entry);
				// Use override weight if set, otherwise use data asset weight
				float Weight = Entry.WeightOverride > 0.0f 
					? Entry.WeightOverride 
					: static_cast<float>(Entry.ObstacleDefinition->SelectionWeight);
				TotalWeight += Weight;
			}
		}
	}

	// If no valid entries and we don't require valid class, return first available
	if (ValidEntries.Num() == 0)
	{
		if (!bRequireValidClass && ObstacleEntries.Num() > 0 && ObstacleEntries[0].ObstacleDefinition)
		{
			return ObstacleEntries[0].ObstacleDefinition;
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
			float Weight = Entry.WeightOverride > 0.0f 
				? Entry.WeightOverride 
				: static_cast<float>(Entry.ObstacleDefinition->SelectionWeight);
			CurrentWeight += Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.ObstacleDefinition;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].ObstacleDefinition;
}

TSubclassOf<AObstacle> UObstacleSpawnComponent::SelectObstacle(EPlayerClass PlayerClass) const
{
	UObstacleDefinition* Definition = SelectObstacleDefinition(PlayerClass);
	return Definition ? Definition->ObstacleClass : nullptr;
}

TArray<TSubclassOf<AObstacle>> UObstacleSpawnComponent::GetValidObstacles(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<AObstacle>> ValidObstacles;

	for (const FObstacleSpawnEntry& Entry : ObstacleEntries)
	{
		if (Entry.ObstacleDefinition && Entry.ObstacleDefinition->ObstacleClass)
		{
			// Check if obstacle allows this player class (use override if set, otherwise check data asset)
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.ObstacleDefinition->AllowedClasses;
			
			if (AllowedClasses.Num() > 0)
			{
				bIsValidForClass = AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidObstacles.Add(Entry.ObstacleDefinition->ObstacleClass);
			}
		}
	}

	return ValidObstacles;
}

















