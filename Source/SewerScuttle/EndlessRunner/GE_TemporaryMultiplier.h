// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_TemporaryMultiplier.generated.h"

/**
 * Base Gameplay Effect for temporary stat multipliers
 * These effects modify multiplier attributes additively and have a duration
 * Multiple effects stack additively (e.g., 2x + 1.5x = 3.5x total)
 */
UCLASS(BlueprintType, Blueprintable)
class SEWERSCUTTLE_API UGE_TemporaryMultiplier : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_TemporaryMultiplier();
};


















