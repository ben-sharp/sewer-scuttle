// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SpawnManager.generated.h"

// Forward declarations
class AEndlessRunnerGameMode;
class ATrackPiece;

/**
 * Manages spawning of collectibles and obstacles on track pieces
 */
UCLASS()
class SEWERSCUTTLE_API USpawnManager : public UObject
{
	GENERATED_BODY()

public:
	USpawnManager();

	/** Initialize spawn manager */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void Initialize(AEndlessRunnerGameMode* InGameMode);

	/** Update spawn manager */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void Update(float DeltaTime);

	/** Reset spawn manager */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void Reset();

	/** Spawn collectibles and obstacles on a track piece */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnOnTrackPiece(ATrackPiece* TrackPiece);

protected:
	/** Game mode reference */
	UPROPERTY()
	AEndlessRunnerGameMode* GameMode;
};

