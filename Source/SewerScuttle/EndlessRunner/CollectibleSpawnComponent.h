// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"
#include "CollectibleDefinition.h"
#include "PlayerClass.h"
#include "CollectibleSpawnComponent.generated.h"

/**
 * Entry for a collectible in the spawn component
 */
USTRUCT(BlueprintType)
struct FCollectibleSpawnEntry
{
	GENERATED_BODY()

	/** Collectible definition data asset (contains class, weight, and properties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	UCollectibleDefinition* CollectibleDefinition;

	/** Override weight from data asset (0 = use data asset weight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ClampMin = "0.0", ClampMax = "100.0", ToolTip = "Override weight from data asset. Set to 0 to use data asset's SelectionWeight."))
	float WeightOverride = 0.0f;

	/** Override allowed classes from data asset (empty = use data asset's allowed classes if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible|Class Restrictions", meta = (ToolTip = "Override allowed classes. Empty = use data asset's allowed classes if any, or allow all classes."))
	TArray<EPlayerClass> AllowedClassesOverride;
};

/**
 * Scene component that can spawn one of multiple collectibles based on player class
 * Attach this to track pieces and it will automatically spawn collectibles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UCollectibleSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCollectibleSpawnComponent();

	/** Select a valid collectible for the given player class using weighted random selection */
	UFUNCTION(BlueprintCallable, Category = "Collectible")
	TSubclassOf<AActor> SelectCollectible(EPlayerClass PlayerClass) const;

	/** Select a collectible definition for the given player class (returns data asset) */
	UFUNCTION(BlueprintCallable, Category = "Collectible")
	UCollectibleDefinition* SelectCollectibleDefinition(EPlayerClass PlayerClass) const;

	/** Get all valid collectibles for the given player class */
	UFUNCTION(BlueprintCallable, Category = "Collectible")
	TArray<TSubclassOf<AActor>> GetValidCollectibles(EPlayerClass PlayerClass) const;

	/** Get the number of collectible entries */
	UFUNCTION(BlueprintPure, Category = "Collectible")
	int32 GetCollectibleEntryCount() const { return CollectibleEntries.Num(); }

	/** Overall chance for this component to spawn anything (0.0 = never, 1.0 = always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Overall chance for this component to spawn anything (0.0 = never, 1.0 = always)"))
	float SpawnChance = 1.0f;

protected:
	/** Array of collectible entries with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ToolTip = "List of collectibles that can spawn at this location"))
	TArray<FCollectibleSpawnEntry> CollectibleEntries;

	/** If true, only spawns collectibles valid for current player class. If false, spawns first available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible", meta = (ToolTip = "If true, only spawns collectibles valid for current player class"))
	bool bRequireValidClass = true;
};

