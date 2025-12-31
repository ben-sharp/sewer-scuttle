// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PowerUp.h"
#include "PowerUpDefinition.h"
#include "PlayerClass.h"
#include "PowerUpSpawnComponent.generated.h"

/**
 * Entry for a power-up in the spawn component
 */
USTRUCT(BlueprintType)
struct FPowerUpSpawnEntry
{
	GENERATED_BODY()

	/** Power-up definition data asset (contains class, weight, and properties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	UPowerUpDefinition* PowerUpDefinition;

	/** Override weight from data asset (0 = use data asset weight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ClampMin = "0.0", ClampMax = "100.0", ToolTip = "Override weight from data asset. Set to 0 to use data asset's SelectionWeight."))
	float WeightOverride = 0.0f;

	/** Override allowed classes from data asset (empty = use data asset's allowed classes if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Class Restrictions", meta = (ToolTip = "Override allowed classes. Empty = use data asset's allowed classes if any, or allow all classes."))
	TArray<EPlayerClass> AllowedClassesOverride;
};

/**
 * Scene component that can spawn one of multiple power-ups based on player class
 * Attach this to track pieces and reference it in spawn points
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UPowerUpSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPowerUpSpawnComponent();

	/** Select a valid power-up for the given player class using weighted random selection */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	TSubclassOf<APowerUp> SelectPowerUp(EPlayerClass PlayerClass) const;

	/** Select a power-up definition for the given player class (returns data asset) */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	UPowerUpDefinition* SelectPowerUpDefinition(EPlayerClass PlayerClass) const;

	/** Get all valid power-ups for the given player class */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	TArray<TSubclassOf<APowerUp>> GetValidPowerUps(EPlayerClass PlayerClass) const;

	/** Get the number of power-up entries */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	int32 GetPowerUpEntryCount() const { return PowerUpEntries.Num(); }

	/** Overall chance for this component to spawn anything (0.0 = never, 1.0 = always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Overall chance for this component to spawn anything (0.0 = never, 1.0 = always)"))
	float SpawnChance = 1.0f;

protected:
	/** Array of power-up entries with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ToolTip = "List of power-ups that can spawn at this location"))
	TArray<FPowerUpSpawnEntry> PowerUpEntries;

	/** If true, only spawns power-ups valid for current player class. If false, spawns first available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ToolTip = "If true, only spawns power-ups valid for current player class"))
	bool bRequireValidClass = true;
};

