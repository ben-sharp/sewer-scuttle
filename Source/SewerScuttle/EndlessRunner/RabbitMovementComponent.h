// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RabbitMovementComponent.generated.h"

/**
 * Custom movement component for rabbit character
 * Handles lane-based movement and constant forward velocity
 */
UCLASS()
class SEWERSCUTTLE_API URabbitMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	URabbitMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Set forward movement speed */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetForwardSpeed(float NewSpeed);

	/** Get forward movement speed */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetForwardSpeed() const { return ForwardSpeed; }

protected:
	/** Constant forward movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float ForwardSpeed = 1000.0f;

	/** Maximum speed for forward movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxForwardSpeed = 2000.0f;
};

