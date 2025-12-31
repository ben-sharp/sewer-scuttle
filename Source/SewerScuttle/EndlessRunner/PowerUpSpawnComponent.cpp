// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUpSpawnComponent.h"
#include "PowerUp.h"
#include "PowerUpDefinition.h"

UPowerUpSpawnComponent::UPowerUpSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UPowerUpDefinition* UPowerUpSpawnComponent::SelectPowerUpDefinition(EPlayerClass PlayerClass) const
{
	TArray<FPowerUpSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid power-ups
	for (const FPowerUpSpawnEntry& Entry : PowerUpEntries)
	{
		if (Entry.PowerUpDefinition && Entry.PowerUpDefinition->PowerUpClass)
		{
			// Check if power-up allows this player class (use override if set, otherwise check data asset)
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.PowerUpDefinition->AllowedClasses;
			
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
					: static_cast<float>(Entry.PowerUpDefinition->SelectionWeight);
				TotalWeight += Weight;
			}
		}
	}

	// If no valid entries and we don't require valid class, return first available
	if (ValidEntries.Num() == 0)
	{
		if (!bRequireValidClass && PowerUpEntries.Num() > 0 && PowerUpEntries[0].PowerUpDefinition)
		{
			return PowerUpEntries[0].PowerUpDefinition;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("PowerUpSpawnComponent: No valid power-ups found for class %d"), (int32)PlayerClass);
		return nullptr;
	}

	// Weighted random selection
	if (TotalWeight > 0.0f)
	{
		float RandomValue = FMath::RandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;

		for (const FPowerUpSpawnEntry& Entry : ValidEntries)
		{
			float Weight = Entry.WeightOverride > 0.0f 
				? Entry.WeightOverride 
				: static_cast<float>(Entry.PowerUpDefinition->SelectionWeight);
			CurrentWeight += Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.PowerUpDefinition;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].PowerUpDefinition;
}

TSubclassOf<APowerUp> UPowerUpSpawnComponent::SelectPowerUp(EPlayerClass PlayerClass) const
{
	UPowerUpDefinition* Definition = SelectPowerUpDefinition(PlayerClass);
	return Definition ? Definition->PowerUpClass : nullptr;
}

TArray<TSubclassOf<APowerUp>> UPowerUpSpawnComponent::GetValidPowerUps(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<APowerUp>> ValidPowerUps;

	for (const FPowerUpSpawnEntry& Entry : PowerUpEntries)
	{
		if (Entry.PowerUpDefinition && Entry.PowerUpDefinition->PowerUpClass)
		{
			// Check if power-up allows this player class
			bool bIsValidForClass = true;
			TArray<EPlayerClass> AllowedClasses = Entry.AllowedClassesOverride.Num() > 0 
				? Entry.AllowedClassesOverride 
				: Entry.PowerUpDefinition->AllowedClasses;
			
			if (AllowedClasses.Num() > 0)
			{
				bIsValidForClass = AllowedClasses.Contains(PlayerClass);
			}

			if (bIsValidForClass)
			{
				ValidPowerUps.Add(Entry.PowerUpDefinition->PowerUpClass);
			}
		}
	}

	return ValidPowerUps;
}

