// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Obstacle.h"
#include "PlayerClass.h"
#include "ObstacleDefinition.generated.h"

/**
 * Data asset defining an obstacle configuration
 * Used to define obstacle properties for content export and server-side validation
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UObstacleDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of this obstacle definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FString ObstacleName;

	/** Blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	TSubclassOf<AObstacle> ObstacleClass;

	/** Obstacle type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	EObstacleType ObstacleType = EObstacleType::Full;

	/** Damage dealt on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Damage = 10.0f;

	/** Lives lost on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "1", ClampMax = "10"))
	int32 LivesLost = 1;

	/** Whether this obstacle can be broken by Enforcer class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsBreakable = false;

	/** If true, bypasses last stand and instantly kills if LivesLost >= player's current lives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIgnoreLastStand = false;

	/** Player classes that can encounter this obstacle (empty = all classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<EPlayerClass> AllowedClasses;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ClampMin = "1"))
	int32 SelectionWeight = 1;
};

