// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseContentDefinition.h"
#include "PlayerClass.h"
#include "GameplayTagContainer.h"
#include "PowerUpDefinition.generated.h"

class APowerUp;

/**
 * Defines properties for a game power-up
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UPowerUpDefinition : public UBaseContentDefinition
{
	GENERATED_BODY()

public:
	virtual EContentType GetContentType() const override { return EContentType::PowerUp; }

	/** Human-readable name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString PowerUpName;

	/** The power-up blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSubclassOf<APowerUp> PowerUpClass;

	/** Duration of the effect in seconds (0 = instant/permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float Duration = 10.0f;

	/** The player stat modified by this power-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	FGameplayTag StatTypeToModify;

	/** Amount to modify the stat by */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float ModificationValue = 1.0f;

	/** GAS effect class for permanent stat modification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<class UGameplayEffect> PermanentStatModifierEffect;

	/** GAS effect class for temporary multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<class UGameplayEffect> TemporaryMultiplierEffect;

	/** Relative probability of being selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SelectionWeight = 1.0f;

    /** Which difficulty tiers this powerup can appear in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop", meta = (ToolTip = "Which tiers this powerup can appear in shops"))
    TArray<ETrackTier> DifficultyAvailability;

    /** Base cost in currency */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    int32 BaseCost = 100;

    /** How much the cost increases per tier (e.g. 0.5 = 50% increase per tier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    float CostMultiplierPerTier = 0.5f;
};

