// Copyright Epic Games, Inc. All Rights Reserved.

#include "CollectibleSpawnComponent.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"

UCollectibleSpawnComponent::UCollectibleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TSubclassOf<AActor> UCollectibleSpawnComponent::SelectCollectible(EPlayerClass PlayerClass) const
{
	TArray<FCollectibleSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid collectibles
	for (const FCollectibleSpawnEntry& Entry : CollectibleEntries)
	{
		if (Entry.CollectibleClass)
		{
			// Check if collectible allows this player class
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
		if (!bRequireValidClass && CollectibleEntries.Num() > 0 && CollectibleEntries[0].CollectibleClass)
		{
			return CollectibleEntries[0].CollectibleClass;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("CollectibleSpawnComponent: No valid collectibles found for class %d"), (int32)PlayerClass);
		return nullptr;
	}

	// Weighted random selection
	if (TotalWeight > 0.0f)
	{
		float RandomValue = FMath::RandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;

		for (const FCollectibleSpawnEntry& Entry : ValidEntries)
		{
			CurrentWeight += Entry.Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.CollectibleClass;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].CollectibleClass;
}

TArray<TSubclassOf<AActor>> UCollectibleSpawnComponent::GetValidCollectibles(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<AActor>> ValidCollectibles;

	for (const FCollectibleSpawnEntry& Entry : CollectibleEntries)
	{
		if (Entry.CollectibleClass)
		{
			// Check if collectible allows this player class
			bool bIsValidForClass = true;
			if (Entry.AllowedClasses.Num() > 0)
			{
				bIsValidForClass = Entry.AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidCollectibles.Add(Entry.CollectibleClass);
			}
		}
	}

	return ValidCollectibles;
}



