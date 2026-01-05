// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "RabbitAttributeSet.h"
#include "Obstacle.h"
#include "RabbitCharacter.generated.h"

class URabbitMovementComponent;
class URabbitJumpComponent;
class URabbitSlideComponent;
class UEndlessRunnerInputConfig;
class UInputAction;
class APowerUp;

UENUM(BlueprintType)
enum class ELanePosition : uint8
{
	Left		UMETA(DisplayName = "Left"),
	Center		UMETA(DisplayName = "Center"),
	Right		UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class ERabbitAnimationState : uint8
{
	Running		UMETA(DisplayName = "Running"),
	Jumping		UMETA(DisplayName = "Jumping"),
	Ducking		UMETA(DisplayName = "Ducking")
};

/**
 * Rabbit character for the endless runner game
 * Features: 3-lane movement, constant forward movement, jump, and slide mechanics
 */
UCLASS()
class SEWERSCUTTLE_API ARabbitCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ARabbitCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Setup Enhanced Input */
	void SetupEnhancedInput();

	/** Get current lane position */
	UFUNCTION(BlueprintPure, Category = "Lane")
	ELanePosition GetCurrentLane() const { return CurrentLane; }

	/** Get target lane position */
	UFUNCTION(BlueprintPure, Category = "Lane")
	ELanePosition GetTargetLane() const { return TargetLane; }

	/** Move to left lane */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void MoveLeft();

	/** Move to right lane */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void MoveRight();

	/** Reset lane position to center (for game start/retry) */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void ResetLanePosition();

	/** Jump action - overrides ACharacter::Jump() */
	void Jump() override;

	/** Slide/Crouch action */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Slide();

	/** Stop sliding */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSlide();

	/** Get forward movement speed */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetForwardSpeed() const { return ForwardSpeed; }

	/** Set forward movement speed */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetForwardSpeed(float NewSpeed);

	/** Get lane width (distance between lanes) */
	UFUNCTION(BlueprintPure, Category = "Lane")
	float GetLaneWidth() const { return LaneWidth; }

	/** Get jump component */
	UFUNCTION(BlueprintPure, Category = "Components")
	URabbitJumpComponent* GetJumpComponent() const { return JumpComponent; }

	/** Get slide component */
	UFUNCTION(BlueprintPure, Category = "Components")
	URabbitSlideComponent* GetSlideComponent() const { return SlideComponent; }

	/** Get current animation state (for anim blueprint) */
	UFUNCTION(BlueprintPure, Category = "Animation")
	ERabbitAnimationState GetAnimationState() const { return AnimationState; }

	/** Check if player is invincible */
	UFUNCTION(BlueprintPure, Category = "Damage")
	bool IsInvincible() const { return bIsInvincible; }

	/** Set invincibility state */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetInvincible(bool bInvincible, float Duration = 0.0f);

	/** Set speed multiplier (affects ForwardSpeed) - updates GAS attribute */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetSpeedMultiplier(float Multiplier);

	/** Get current speed multiplier from GAS */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetSpeedMultiplier() const;

	/** Enable/disable autopilot mode */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void SetAutopilot(bool bEnabled);

	/** Check if autopilot is active */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsAutopilotActive() const { return bAutopilotActive; }

	/** Enable ragdoll physics and launch character into the air */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void EnableRagdollDeath(const FVector& LaunchVelocity);

	/** Reset ragdoll state and restore normal character movement */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void ResetRagdollState();

	/** Set maximum jump count (for multi-jump classes) */
	UFUNCTION(BlueprintCallable, Category = "Class")
	void SetMaxJumpCount(int32 Count);

	/** Set capsule size (for Scout class) */
	UFUNCTION(BlueprintCallable, Category = "Class")
	void SetCapsuleSize(float HalfHeight, float Radius);

	/** Set never needs crouch flag (for Scout class) */
	UFUNCTION(BlueprintCallable, Category = "Class")
	void SetNeverNeedsCrouch(bool bNeverNeeds);

	/** Set can break obstacles flag (for Enforcer class) */
	UFUNCTION(BlueprintCallable, Category = "Class")
	void SetCanBreakObstacles(bool bCanBreak);

	/** Set lane change responsiveness */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void SetLaneChangeResponsiveness(float NewResponsiveness) { LaneChangeResponsiveness = NewResponsiveness; }

	/** Check if never needs crouch */
	UFUNCTION(BlueprintPure, Category = "Class")
	bool GetNeverNeedsCrouch() const { return bNeverNeedsCrouch; }

	/** Check if can break obstacles */
	UFUNCTION(BlueprintPure, Category = "Class")
	bool GetCanBreakObstacles() const { return bCanBreakObstacles; }

	/** Fixed Y coordinates for each lane (locked in for flat track pieces) */
	static constexpr float LANE_LEFT_Y = -200.0f;   // Left lane fixed Y coordinate
	static constexpr float LANE_CENTER_Y = 0.0f;    // Center lane fixed Y coordinate
	static constexpr float LANE_RIGHT_Y = 200.0f;   // Right lane fixed Y coordinate

	/** Input action handlers */
	void OnMoveLeft(const FInputActionValue& Value);
	void OnMoveRight(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);
	void OnSlide(const FInputActionValue& Value);
	void OnSlideReleased(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Width of each lane - distance between lane centers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float LaneWidth = 200.0f;

	/** Speed of lane transition (can be modified by powerups) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float LaneTransitionSpeed = 10.0f;

	/** How responsive lane changes feel (1.0 = normal, higher = more mashable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane")
	float LaneChangeResponsiveness = 1.0f;

	/** Base lane transition speed (for powerup reset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float BaseLaneTransitionSpeed = 10.0f;

	/** Get lane transition speed */
	UFUNCTION(BlueprintPure, Category = "Lane")
	float GetLaneTransitionSpeed() const { return LaneTransitionSpeed; }

	/** Set lane transition speed (for powerups) */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void SetLaneTransitionSpeed(float NewSpeed) { LaneTransitionSpeed = FMath::Clamp(NewSpeed, 1.0f, 50.0f); }

	/** Reset lane transition speed to base (for powerup expiration) */
	UFUNCTION(BlueprintCallable, Category = "Lane")
	void ResetLaneTransitionSpeed() { LaneTransitionSpeed = BaseLaneTransitionSpeed; }

	/** Base forward movement speed (before multipliers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float BaseForwardSpeed = 1000.0f;

	/** Current forward movement speed (with multipliers applied) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float ForwardSpeed = 1000.0f;

	/** Toggle for lane and buffer debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane|Debug")
	bool bShowLaneDebug = false;

	/** Current responsiveness value synced from GAS */
	float CurrentResponsiveness = 1.0f;

	/** Draw debug visualization for lanes and input buffers */
	void DrawLaneDebugVisualization();

	/** Current lane position */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane")
	ELanePosition CurrentLane = ELanePosition::Center;

	/** Target lane position (for smooth transitions) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane")
	ELanePosition TargetLane = ELanePosition::Center;

	/** Current X position in lane (for smooth transitions) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane")
	float CurrentLaneX = 0.0f;

	/** Custom movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URabbitMovementComponent* RabbitMovementComponent;

	/** Jump component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URabbitJumpComponent* JumpComponent;

	/** Slide component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URabbitSlideComponent* SlideComponent;

	/** Input configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UEndlessRunnerInputConfig* InputConfig;

	/** Current animation state (for anim blueprint) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	ERabbitAnimationState AnimationState = ERabbitAnimationState::Running;

	/** Invincibility state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	bool bIsInvincible = false;

	/** Invincibility timer handle */
	FTimerHandle InvincibilityTimerHandle;

	/** Autopilot state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	bool bAutopilotActive = false;

	/** Never needs to crouch (Scout class) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class")
	bool bNeverNeedsCrouch = false;

	/** Can break through breakable obstacles (Enforcer class) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Class")
	bool bCanBreakObstacles = false;

private:
	/** Update lane position smoothly */
	void UpdateLanePosition(float DeltaTime);

	/** Check if we can initiate a new lane change (dynamic throttle) */
	bool CanInitiateLaneChange() const;

	/** Get target X position for a lane */
	float GetLaneXPosition(ELanePosition Lane) const;

	/** Update animation state based on character movement */
	void UpdateAnimationState();

	/** Called when invincibility expires */
	void OnInvincibilityExpired();

	/** Update autopilot - automatically dodge obstacles */
	void UpdateAutopilot(float DeltaTime);

	/** Check if a lane is safe (no obstacles ahead) */
	bool IsLaneSafe(ELanePosition Lane, float LookAheadDistance = 1000.0f) const;

	/** Check if a lane has a power-up ahead */
	bool HasPowerUpAhead(ELanePosition Lane, float LookAheadDistance = 50.0f) const;

	/** Check if a lane has a specific obstacle type ahead */
	bool HasObstacleTypeAhead(ELanePosition Lane, EObstacleType ObstacleType, float LookAheadDistance = 50.0f) const;

	/** Get the safest lane to move to */
	ELanePosition GetSafestLane() const;

	// GAS Interface
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Get attribute set */
	UFUNCTION(BlueprintPure, Category = "GAS")
	URabbitAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Get current speed from GAS */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetCurrentSpeed() const;

	/** Get current jump height from GAS */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetCurrentJumpHeight() const;

	/** Get current max jump count from GAS */
	UFUNCTION(BlueprintPure, Category = "Movement")
	int32 GetCurrentMaxJumpCount() const;

	/** Get current lives from GAS */
	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentLives() const;

public:
	/** Get coin multiplier from GAS */
	UFUNCTION(BlueprintPure, Category = "Utility")
	float GetCoinMultiplier() const;

	/** Get score multiplier from GAS */
	UFUNCTION(BlueprintPure, Category = "Utility")
	float GetScoreMultiplier() const;

	/** Check if magnet is active from GAS */
	UFUNCTION(BlueprintPure, Category = "Special")
	bool IsMagnetActiveFromGAS() const;

	/** Check if autopilot is active from GAS */
	UFUNCTION(BlueprintPure, Category = "Special")
	bool IsAutopilotActiveFromGAS() const;

	/** Check if invincibility is active from GAS */
	UFUNCTION(BlueprintPure, Category = "Special")
	bool IsInvincibilityActiveFromGAS() const;

	/** Reset all GAS effects and attributes to default values (called on retry/reset) */
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void ResetGASEffects();

	/** Apply an effect based on Gameplay Tag and value (from shop items) */
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void ApplyEffectByTag(FGameplayTag Tag, float Value);

protected:
	/** Ability System Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute Set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	URabbitAttributeSet* AttributeSet;
};

