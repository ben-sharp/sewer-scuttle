// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_BaseStatModifier.generated.h"

/**
 * Base Gameplay Effect for permanent base stat modifications
 * These effects modify base attributes additively and last for the duration of the run
 */
UCLASS(BlueprintType, Blueprintable)
class SEWERSCUTTLE_API UGE_BaseStatModifier : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_BaseStatModifier();
};


















