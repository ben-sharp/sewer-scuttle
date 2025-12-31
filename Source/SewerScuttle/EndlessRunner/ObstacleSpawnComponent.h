// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Obstacle.h"
#include "ObstacleDefinition.h"
#include "PlayerClass.h"
#include "ObstacleSpawnComponent.generated.h"

/**
 * Entry for an obstacle in the spawn component
 */
USTRUCT(BlueprintType)
struct FObstacleSpawnEntry
{
	GENERATED_BODY()

	/** Obstacle definition data asset (contains class, weight, and properties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	UObstacleDefinition* ObstacleDefinition;

	/** Override weight from data asset (0 = use data asset weight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ClampMin = "0.0", ClampMax = "100.0", ToolTip = "Override weight from data asset. Set to 0 to use data asset's SelectionWeight."))
	float WeightOverride = 0.0f;

	/** Override allowed classes from data asset (empty = use data asset's allowed classes if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Class Restrictions", meta = (ToolTip = "Override allowed classes. Empty = use data asset's allowed classes if any, or allow all classes."))
	TArray<EPlayerClass> AllowedClassesOverride;
};

/**
 * Scene component that can spawn one of multiple obstacles based on player class
 * Attach this to track pieces and reference it in spawn points
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UObstacleSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UObstacleSpawnComponent();

	/** Select a valid obstacle for the given player class using weighted random selection */
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
	TSubclassOf<AObstacle> SelectObstacle(EPlayerClass PlayerClass) const;

	/** Select an obstacle definition for the given player class (returns data asset) */
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
	UObstacleDefinition* SelectObstacleDefinition(EPlayerClass PlayerClass) const;

	/** Get all valid obstacles for the given player class */
	UFUNCTION(BlueprintCallable, Category = "Obstacle")
	TArray<TSubclassOf<AObstacle>> GetValidObstacles(EPlayerClass PlayerClass) const;

	/** Get the number of obstacle entries */
	UFUNCTION(BlueprintPure, Category = "Obstacle")
	int32 GetObstacleEntryCount() const { return ObstacleEntries.Num(); }

	/** Overall chance for this component to spawn anything (0.0 = never, 1.0 = always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Overall chance for this component to spawn anything (0.0 = never, 1.0 = always)"))
	float SpawnChance = 1.0f;

protected:
	/** Array of obstacle entries with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ToolTip = "List of obstacles that can spawn at this location"))
	TArray<FObstacleSpawnEntry> ObstacleEntries;

	/** If true, only spawns obstacles valid for current player class. If false, spawns first available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle", meta = (ToolTip = "If true, only spawns obstacles valid for current player class"))
	bool bRequireValidClass = true;
};

