// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUpSpawnComponent.h"

UPowerUpSpawnComponent::UPowerUpSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default settings
	SpawnProbability = 0.5f;
}
