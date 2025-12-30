// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUpSpawnComponent.h"
#include "PowerUp.h"

UPowerUpSpawnComponent::UPowerUpSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TSubclassOf<APowerUp> UPowerUpSpawnComponent::SelectPowerUp(EPlayerClass PlayerClass) const
{
	TArray<FPowerUpSpawnEntry> ValidEntries;
	float TotalWeight = 0.0f;

	// Filter valid power-ups
	for (const FPowerUpSpawnEntry& Entry : PowerUpEntries)
	{
		if (Entry.PowerUpClass)
		{
			// Check if power-up class allows this player class
			APowerUp* CDO = Entry.PowerUpClass->GetDefaultObject<APowerUp>();
			if (CDO && CDO->IsValidForClass(PlayerClass))
			{
				ValidEntries.Add(Entry);
				TotalWeight += Entry.Weight;
			}
		}
	}

	// If no valid entries and we don't require valid class, return first available
	if (ValidEntries.Num() == 0)
	{
		if (!bRequireValidClass && PowerUpEntries.Num() > 0 && PowerUpEntries[0].PowerUpClass)
		{
			return PowerUpEntries[0].PowerUpClass;
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
			CurrentWeight += Entry.Weight;
			if (RandomValue <= CurrentWeight)
			{
				return Entry.PowerUpClass;
			}
		}
	}

	// Fallback to first valid entry
	return ValidEntries[0].PowerUpClass;
}

TArray<TSubclassOf<APowerUp>> UPowerUpSpawnComponent::GetValidPowerUps(EPlayerClass PlayerClass) const
{
	TArray<TSubclassOf<APowerUp>> ValidPowerUps;

	for (const FPowerUpSpawnEntry& Entry : PowerUpEntries)
	{
		if (Entry.PowerUpClass)
		{
			APowerUp* CDO = Entry.PowerUpClass->GetDefaultObject<APowerUp>();
			if (CDO && CDO->IsValidForClass(PlayerClass))
			{
				ValidPowerUps.Add(Entry.PowerUpClass);
			}
		}
	}

	return ValidPowerUps;
}

