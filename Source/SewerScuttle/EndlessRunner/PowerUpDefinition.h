// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PowerUp.h"
#include "PlayerClass.h"
#include "PowerUpDefinition.generated.h"

/**
 * Data asset defining a powerup configuration
 * Used to define powerup properties for content export and server-side validation
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UPowerUpDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of this powerup definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FString PowerUpName;

	/** Blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	TSubclassOf<APowerUp> PowerUpClass;

	/** Duration of powerup effect in seconds (0 = instant/one-time effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float Duration = 5.0f;

	/** Player classes that can use this power-up (empty = all classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<EPlayerClass> AllowedClasses;

	/** Gameplay Effect class for permanent base stat modification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> PermanentStatModifierEffect;

	/** Gameplay Effect class for temporary multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> TemporaryMultiplierEffect;

	/** Stat type to modify (e.g., "BaseSpeed", "SpeedMultiplier", "CoinMultiplier") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FName StatTypeToModify;

	/** Modification value (additive for permanent, multiplier for temporary) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	float ModificationValue = 0.0f;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ClampMin = "1"))
	int32 SelectionWeight = 1;
};

