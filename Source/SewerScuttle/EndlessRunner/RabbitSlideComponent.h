// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RabbitSlideComponent.generated.h"

/**
 * Component handling slide/crouch mechanics for the rabbit character
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API URabbitSlideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URabbitSlideComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Start sliding */
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void StartSlide();

	/** Stop sliding */
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void StopSlide();

	/** Check if character is currently sliding */
	UFUNCTION(BlueprintPure, Category = "Slide")
	bool IsSliding() const { return bIsSliding; }

	/** Perform stomp/drop when in air */
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void PerformStomp();

protected:
	/** Slide duration in seconds (0 = hold to slide) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SlideDuration = 0.0f; // 0 means hold to slide

	/** Slide height (capsule half height during slide) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "20.0", ClampMax = "100.0"))
	float SlideHeight = 44.0f;

	/** Original capsule half height */
	float OriginalCapsuleHalfHeight = 88.0f;

	/** Slide transition speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float SlideTransitionSpeed = 10.0f;

	/** Cooldown after slide ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SlideCooldown = 0.1f;

	/** Stomp/drop velocity (negative = downward) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide", meta = (ClampMin = "-5000.0", ClampMax = "-100.0", ToolTip = "Downward velocity applied when stomping in air"))
	float StompVelocity = -2000.0f;

private:
	/** Whether character is currently sliding */
	bool bIsSliding = false;

	/** Time remaining in current slide */
	float SlideTimeRemaining = 0.0f;

	/** Time until next slide is allowed */
	float CooldownRemaining = 0.0f;

	/** Target capsule half height */
	float TargetCapsuleHalfHeight = 88.0f;

	/** Update slide state */
	void UpdateSlide(float DeltaTime);

	/** Get capsule component */
	class UCapsuleComponent* GetCapsuleComponent() const;
};

