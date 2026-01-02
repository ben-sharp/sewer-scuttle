// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObstacleSpawnComponent.h"

UObstacleSpawnComponent::UObstacleSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default settings
	SpawnProbability = 1.0f;
}
