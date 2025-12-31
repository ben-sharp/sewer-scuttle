// Copyright Epic Games, Inc. All Rights Reserved.

#include "CollectibleSpawnComponent.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"
#include "CollectibleDefinition.h"

UCollectibleSpawnComponent::UCollectibleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UCollectibleDefinition* UCollectibleSpawnComponent::SelectCollectibleDefinition(EPlayerClass PlayerClass) const
{
	TArray<FCollectibleSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid collectibles
	for (const FCollectibleSpawnEntry& Entry : CollectibleEntries)
	{
		if (Entry.CollectibleDefinition && Entry.CollectibleDefinition->CollectibleClass)
		{
			// Check if collectible allows this player class (use override if set, otherwise check data asset)
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.CollectibleDefinition->AllowedClasses;
			
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
					: static_cast<float>(Entry.CollectibleDefinition->SelectionWeight);
				TotalWeight += Weight;
			}
		}
	}

	// If no valid entries and we don't require valid class, return first available
	if (ValidEntries.Num() == 0)
	{
		if (!bRequireValidClass && CollectibleEntries.Num() > 0 && CollectibleEntries[0].CollectibleDefinition)
		{
			return CollectibleEntries[0].CollectibleDefinition;
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
			float Weight = Entry.WeightOverride > 0.0f 
				? Entry.WeightOverride 
				: static_cast<float>(Entry.CollectibleDefinition->SelectionWeight);
			CurrentWeight += Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.CollectibleDefinition;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].CollectibleDefinition;
}

TSubclassOf<AActor> UCollectibleSpawnComponent::SelectCollectible(EPlayerClass PlayerClass) const
{
	UCollectibleDefinition* Definition = SelectCollectibleDefinition(PlayerClass);
	return Definition ? Definition->CollectibleClass : nullptr;
}

TArray<TSubclassOf<AActor>> UCollectibleSpawnComponent::GetValidCollectibles(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<AActor>> ValidCollectibles;

	for (const FCollectibleSpawnEntry& Entry : CollectibleEntries)
	{
		if (Entry.CollectibleDefinition && Entry.CollectibleDefinition->CollectibleClass)
		{
			// Check if collectible allows this player class (use override if set, otherwise check data asset)
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.CollectibleDefinition->AllowedClasses;
			
			if (AllowedClasses.Num() > 0)
			{
				bIsValidForClass = AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidCollectibles.Add(Entry.CollectibleDefinition->CollectibleClass);
			}
		}
	}

	return ValidCollectibles;
}

















