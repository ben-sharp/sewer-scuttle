// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseContentDefinition.h"
#include "Obstacle.h"
#include "PlayerClass.h"
#include "ObstacleDefinition.generated.h"

class AObstacle;

/**
 * Defines properties for a game obstacle
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UObstacleDefinition : public UBaseContentDefinition
{
	GENERATED_BODY()

public:
	virtual EContentType GetContentType() const override { return EContentType::Obstacle; }

	/** Human-readable name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString ObstacleName;

	/** The obstacle blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSubclassOf<AObstacle> ObstacleClass;

	/** Type of obstacle (Low, High, Full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	EObstacleType ObstacleType = EObstacleType::Full;

	/** Base damage dealt to player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 Damage = 1;

	/** Number of lives lost on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 LivesLost = 1;

	/** Can this obstacle be broken by certain classes? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsBreakable = false;

	/** Should this obstacle bypass last stand protection? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIgnoreLastStand = false;

	/** Relative probability of being selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SelectionWeight = 1.0f;
};

