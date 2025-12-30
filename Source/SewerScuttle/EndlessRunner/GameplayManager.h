// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayManager.generated.h"

class AEndlessRunnerGameMode;
class USpawnManager;

/**
 * Manages gameplay difficulty and spawn rates
 * Scales difficulty based on distance/time
 */
UCLASS()
class SEWERSCUTTLE_API UGameplayManager : public UObject
{
	GENERATED_BODY()

public:
	UGameplayManager();

	/** Initialize gameplay manager */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Initialize(AEndlessRunnerGameMode* InGameMode);

	/** Update gameplay manager */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Update(float DeltaTime, float DistanceTraveled);

	/** Reset gameplay manager */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void Reset();

	/** Get current difficulty level */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetCurrentDifficulty() const { return CurrentDifficulty; }

	/** Get coin spawn probability */
	UFUNCTION(BlueprintPure, Category = "Spawning")
	float GetCoinSpawnProbability() const { return CoinSpawnProbability; }

	/** Get powerup spawn probability */
	UFUNCTION(BlueprintPure, Category = "Spawning")
	float GetPowerUpSpawnProbability() const { return PowerUpSpawnProbability; }

	/** Get obstacle spawn probability */
	UFUNCTION(BlueprintPure, Category = "Spawning")
	float GetObstacleSpawnProbability() const { return ObstacleSpawnProbability; }

	/** Get spawn manager */
	UFUNCTION(BlueprintPure, Category = "Spawning")
	USpawnManager* GetSpawnManager() const { return SpawnManager; }

protected:
	/** Game mode reference */
	UPROPERTY()
	AEndlessRunnerGameMode* GameMode;

	/** Spawn manager */
	UPROPERTY()
	USpawnManager* SpawnManager;

	/** Current difficulty level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Difficulty")
	int32 CurrentDifficulty = 0;

	/** Distance at which difficulty increases */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float DifficultyIncreaseDistance = 1000.0f;

	/** Base coin spawn probability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseCoinSpawnProbability = 0.3f;

	/** Base powerup spawn probability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BasePowerUpSpawnProbability = 0.1f;

	/** Base obstacle spawn probability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseObstacleSpawnProbability = 0.2f;

	/** Current coin spawn probability */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	float CoinSpawnProbability = 0.3f;

	/** Current powerup spawn probability */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	float PowerUpSpawnProbability = 0.1f;

	/** Current obstacle spawn probability */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	float ObstacleSpawnProbability = 0.2f;

	/** Update difficulty based on distance */
	void UpdateDifficulty(float DistanceTraveled);

	/** Update spawn probabilities based on difficulty */
	void UpdateSpawnProbabilities();
};

