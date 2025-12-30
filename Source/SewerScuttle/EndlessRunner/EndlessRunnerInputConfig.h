// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "EndlessRunnerInputConfig.generated.h"

/**
 * Input configuration data asset for endless runner
 * Defines all input actions and their mappings
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UEndlessRunnerInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Input action for moving left */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveLeftAction;

	/** Input action for moving right */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveRightAction;

	/** Input action for jumping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	/** Input action for sliding */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideAction;

	/** Input action for pausing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseAction;
};

