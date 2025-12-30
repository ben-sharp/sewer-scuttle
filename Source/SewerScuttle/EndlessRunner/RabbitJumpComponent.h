// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RabbitAttributeSet.h"
#include "RabbitJumpComponent.generated.h"

/**
 * Component handling jump mechanics for the rabbit character
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API URabbitJumpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URabbitJumpComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Perform jump action */
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void PerformJump();

	/** Check if character is currently jumping */
	UFUNCTION(BlueprintPure, Category = "Jump")
	bool IsJumping() const { return bIsJumping; }

	/** Get jump height (from GAS if available, otherwise from local property) */
	UFUNCTION(BlueprintPure, Category = "Jump")
	float GetJumpHeight() const;

	/** Set maximum jump count (for multi-jump) */
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void SetMaxJumpCount(int32 Count);

	/** Get maximum jump count (from GAS if available, otherwise from local property) */
	UFUNCTION(BlueprintPure, Category = "Jump")
	int32 GetMaxJumpCount() const;

	/** Get current jump count (jumps used in current air time) */
	UFUNCTION(BlueprintPure, Category = "Jump")
	int32 GetCurrentJumpCount() const { return CurrentJumpCount; }

	/** Reset jump count (called when landing) */
	UFUNCTION(BlueprintCallable, Category = "Jump")
	void ResetJumpCount() { CurrentJumpCount = 0; }

protected:
	/** Jump height in units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
	float JumpHeight = 300.0f;

	/** Velocity multiplier for multi-jumps (reduces jump height for 2nd, 3rd, etc. jumps) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MultiJumpVelocityMultiplier = 0.7f;

	/** Jump duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float JumpDuration = 0.5f;

	/** Cooldown between jumps in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float JumpCooldown = 0.1f;

	/** Maximum number of jumps allowed (1 = single jump, 2 = double jump, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxJumpCount = 1;

private:
	/** Whether character is currently jumping */
	bool bIsJumping = false;

	/** Current jump count (tracks jumps used in current air time) */
	int32 CurrentJumpCount = 0;

	/** Time remaining in current jump */
	float JumpTimeRemaining = 0.0f;

	/** Time until next jump is allowed */
	float CooldownRemaining = 0.0f;

	/** Handle jump logic */
	void UpdateJump(float DeltaTime);

	/** Get character movement component */
	class UCharacterMovementComponent* GetCharacterMovement() const;
};

