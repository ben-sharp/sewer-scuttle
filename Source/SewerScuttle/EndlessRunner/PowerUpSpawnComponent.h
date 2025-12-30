// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PowerUp.h"
#include "PlayerClass.h"
#include "PowerUpSpawnComponent.generated.h"

/**
 * Entry for a power-up in the spawn component
 */
USTRUCT(BlueprintType)
struct FPowerUpSpawnEntry
{
	GENERATED_BODY()

	/** Power-up class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	TSubclassOf<APowerUp> PowerUpClass;

	/** Weight for weighted random selection (default: 1.0 = equal chance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float Weight = 1.0f;

	/** Display name for editor (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
	FString DisplayName;
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

