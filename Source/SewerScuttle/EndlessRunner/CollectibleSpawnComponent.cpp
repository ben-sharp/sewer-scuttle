// Copyright Epic Games, Inc. All Rights Reserved.

#include "CollectibleSpawnComponent.h"

UCollectibleSpawnComponent::UCollectibleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default settings
	SpawnProbability = 1.0f;
}
