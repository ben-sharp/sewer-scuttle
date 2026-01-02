// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitCharacter.h"
#include "RabbitMovementComponent.h"
#include "RabbitJumpComponent.h"
#include "RabbitSlideComponent.h"
#include "EndlessRunnerInputConfig.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "TimerManager.h"
#include "Obstacle.h"
#include "PowerUp.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "../UI/EndlessRunnerHUD.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

ARabbitCharacter::ARabbitCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URabbitMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
	GetCapsuleComponent()->SetCapsuleRadius(34.0f);

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
		MovementComp->JumpZVelocity = 600.0f;
		MovementComp->AirControl = 0.35f;
		ForwardSpeed = BaseForwardSpeed; // Will be updated from GAS in BeginPlay
		MovementComp->MaxWalkSpeed = ForwardSpeed;
		MovementComp->MinAnalogWalkSpeed = 20.0f;
		MovementComp->BrakingDecelerationWalking = 2000.0f;
		MovementComp->GravityScale = 1.0f;
		// Always allow running - character should always be moving forward
		MovementComp->bCanWalkOffLedges = true;
		MovementComp->bCanWalkOffLedgesWhenCrouching = true;
	}

	// Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Get custom movement component
	RabbitMovementComponent = Cast<URabbitMovementComponent>(GetCharacterMovement());

	// Create jump component
	JumpComponent = CreateDefaultSubobject<URabbitJumpComponent>(TEXT("JumpComponent"));

	// Create slide component
	SlideComponent = CreateDefaultSubobject<URabbitSlideComponent>(TEXT("SlideComponent"));

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<URabbitAttributeSet>(TEXT("AttributeSet"));
}

void ARabbitCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize GAS
	if (AbilitySystemComponent && AttributeSet)
	{
		// Initialize attribute set with default values
		// The attribute set constructor already sets default values, but we can override here if needed
		
		// Set base speed from character's BaseForwardSpeed
		AttributeSet->SetBaseSpeed(BaseForwardSpeed);
		AttributeSet->SetBaseLaneTransitionSpeed(BaseLaneTransitionSpeed);
		
		UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: GAS initialized with BaseSpeed=%.2f"), BaseForwardSpeed);
	}

	// Ensure tick is enabled
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;
	
	// Check world pause state
	if (UWorld* World = GetWorld())
	{
		bool bIsPaused = World->IsPaused();
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter BeginPlay: World paused=%d"), bIsPaused ? 1 : 0);
	}

	// Initialize lane position
	CurrentLane = ELanePosition::Center;
	TargetLane = ELanePosition::Center;
	CurrentLaneX = GetLaneXPosition(CurrentLane); // This should be 0.0f for center lane
	
	// Initialize base lane transition speed
	BaseLaneTransitionSpeed = LaneTransitionSpeed;

	// Set initial position to start of track at center lane
	// X = 100 (100 units forward to avoid immediate collisions), Y = 0 (center lane - fixed coordinate), Z = 200 (above ground)
	// This matches where the first track piece spawns (X=0, Y=0, Z=0) but we spawn forward
	FVector InitialLocation = GetActorLocation();
	InitialLocation.X = 100.0f;  // Start 100 units forward to avoid immediate collisions
	InitialLocation.Y = ARabbitCharacter::LANE_CENTER_Y;  // Center lane (0.0f)
	InitialLocation.Z = 200.0f;  // Above ground
	SetActorLocation(InitialLocation, false, nullptr, ETeleportType::TeleportPhysics);
	
	// Ensure CurrentLaneX matches the actual position
	CurrentLaneX = ARabbitCharacter::LANE_CENTER_Y;
	
	UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter BeginPlay: spawned at (%.2f, %.2f, %.2f), TickEnabled=%d, CanEverTick=%d"), 
		InitialLocation.X, InitialLocation.Y, InitialLocation.Z, IsActorTickEnabled(), PrimaryActorTick.bCanEverTick);

	// Set initial forward movement
	if (RabbitMovementComponent)
	{
		RabbitMovementComponent->SetForwardSpeed(ForwardSpeed);
	}
	
	// Set initial velocity immediately
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		FVector ForwardDir = FVector(1.0f, 0.0f, 0.0f);
		MovementComp->Velocity = ForwardDir * ForwardSpeed;
		MovementComp->SetMovementMode(MOVE_Walking);
		MovementComp->UpdateComponentVelocity();
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter BeginPlay: Set initial velocity to (%.2f, %.2f, %.2f)"), 
			MovementComp->Velocity.X, MovementComp->Velocity.Y, MovementComp->Velocity.Z);
	}

	// Initialize animation state to running
	AnimationState = ERabbitAnimationState::Running;

	// Setup Enhanced Input
	SetupEnhancedInput();
	
	// Start with invincibility disabled - will be enabled by GameMode when game starts
	bIsInvincible = false;
}

void ARabbitCharacter::Tick(float DeltaTime)
{
	// Debug logging disabled for performance
	// static int32 TickCallCount = 0;
	// TickCallCount++;
	// if (TickCallCount <= 5)
	// {
	// 	UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter Tick: Count=%d"), TickCallCount);
	// }
	
	Super::Tick(DeltaTime);

	// Update autopilot if active
	if (bAutopilotActive)
	{
		UpdateAutopilot(DeltaTime);
	}

	// Update lane position smoothly
	UpdateLanePosition(DeltaTime);

	// Reset jump count when landing (for multi-jump)
	if (JumpComponent && GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
	{
		JumpComponent->ResetJumpCount();
	}

	// Apply constant forward movement (always running unless jumping)
	if (RabbitMovementComponent && GetCharacterMovement())
	{
		// Use fixed forward direction (X axis) for endless runner
		// Character should always move forward along X axis regardless of rotation
		FVector ForwardDirection = FVector(1.0f, 0.0f, 0.0f);
		
		// Debug logging (disabled for performance - only enable when debugging)
		// static int32 TickCount = 0;
		// TickCount++;
		// if (TickCount <= 10 || TickCount % 300 == 0) // Log first 10 ticks, then every 5 seconds at 60fps
		// {
		// 	UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter Tick %d"), TickCount);
		// }
		
		// Update ForwardSpeed from GAS
		if (AttributeSet)
		{
			ForwardSpeed = AttributeSet->GetCurrentSpeed();
			LaneTransitionSpeed = AttributeSet->GetCurrentLaneTransitionSpeed();
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->GravityScale = AttributeSet->GetCurrentGravityScale();
			}
		}
		
		// Always apply forward movement input
		AddMovementInput(ForwardDirection, 1.0f);
		
		// ALWAYS force velocity directly - don't rely on movement input
		// This ensures the character moves even if there are issues with movement input processing
		FVector DesiredVelocity = ForwardDirection * ForwardSpeed;
		if (GetCharacterMovement()->Velocity.SizeSquared() < DesiredVelocity.SizeSquared() * 0.9f)
		{
			// Only update if velocity is significantly less than desired
			GetCharacterMovement()->Velocity.X = DesiredVelocity.X;
			GetCharacterMovement()->Velocity.Y = DesiredVelocity.Y;
			// Don't override Z velocity (for jumping/falling)
			GetCharacterMovement()->UpdateComponentVelocity();
			
			// Debug logging disabled for performance
			// if (TickCount <= 10)
			// {
			// 	UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Forced velocity"));
			// }
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RabbitCharacter: RabbitMovementComponent or GetCharacterMovement() is null!"));
	}

	// Update animation state
	UpdateAnimationState();
}

void ARabbitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputConfig)
		{
			// Bind input actions - use Started for lane switching (one-time per press, not held)
			if (InputConfig->MoveLeftAction)
			{
				EnhancedInputComponent->BindAction(InputConfig->MoveLeftAction, ETriggerEvent::Started, this, &ARabbitCharacter::OnMoveLeft);
			}

			if (InputConfig->MoveRightAction)
			{
				EnhancedInputComponent->BindAction(InputConfig->MoveRightAction, ETriggerEvent::Started, this, &ARabbitCharacter::OnMoveRight);
			}

			if (InputConfig->JumpAction)
			{
				EnhancedInputComponent->BindAction(InputConfig->JumpAction, ETriggerEvent::Triggered, this, &ARabbitCharacter::OnJump);
			}

			if (InputConfig->SlideAction)
			{
				EnhancedInputComponent->BindAction(InputConfig->SlideAction, ETriggerEvent::Triggered, this, &ARabbitCharacter::OnSlide);
				EnhancedInputComponent->BindAction(InputConfig->SlideAction, ETriggerEvent::Completed, this, &ARabbitCharacter::OnSlideReleased);
			}

			if (InputConfig->PauseAction)
			{
				EnhancedInputComponent->BindAction(InputConfig->PauseAction, ETriggerEvent::Started, this, &ARabbitCharacter::OnPause);
			}
		}
	}
}

void ARabbitCharacter::SetupEnhancedInput()
{
	// Enhanced Input setup will be configured in Blueprint/Project Settings
	// The input actions are bound in SetupPlayerInputComponent
	// This method is a placeholder for future Enhanced Input subsystem configuration
}

void ARabbitCharacter::MoveLeft()
{
	// Dynamic throttle check
	if (!CanInitiateLaneChange())
	{
		return;
	}

	// Move one lane to the left from our current TARGET
	// This allows smooth sequential lane changes without snapping
	if (TargetLane == ELanePosition::Right)
	{
		TargetLane = ELanePosition::Center;
	}
	else if (TargetLane == ELanePosition::Center)
	{
		TargetLane = ELanePosition::Left;
	}
}

void ARabbitCharacter::MoveRight()
{
	// Dynamic throttle check
	if (!CanInitiateLaneChange())
	{
		return;
	}

	// Move one lane to the right from our current TARGET
	// This allows smooth sequential lane changes without snapping
	if (TargetLane == ELanePosition::Left)
	{
		TargetLane = ELanePosition::Center;
	}
	else if (TargetLane == ELanePosition::Center)
	{
		TargetLane = ELanePosition::Right;
	}
}

void ARabbitCharacter::ResetLanePosition()
{
	CurrentLane = ELanePosition::Center;
	TargetLane = ELanePosition::Center;
	CurrentLaneX = GetLaneXPosition(ELanePosition::Center);
	
	// Update actor position to match center lane
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Y = CurrentLaneX;
	SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ARabbitCharacter::Jump()
{
	// Only call Super::Jump() - don't call PerformJump() here to avoid circular calls
	// PerformJump() will be called from OnJump() if needed, or we can track jump state separately
	Super::Jump();
}

void ARabbitCharacter::Slide()
{
	if (SlideComponent)
	{
		SlideComponent->StartSlide();
	}
}

void ARabbitCharacter::StopSlide()
{
	if (SlideComponent)
	{
		SlideComponent->StopSlide();
	}
}

void ARabbitCharacter::UpdateLanePosition(float DeltaTime)
{
	float TargetY = GetLaneXPosition(TargetLane);  // Note: GetLaneXPosition returns Y coordinate (naming is historical)
	
	// Smoothly interpolate to target position
	CurrentLaneX = FMath::FInterpTo(CurrentLaneX, TargetY, DeltaTime, LaneTransitionSpeed);

	// Update actor position - lock Y coordinate to fixed lane position
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Y = CurrentLaneX; // This is now a fixed coordinate from GetLaneXPosition
	SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::None);

	// Update current lane if we've reached the target
	if (FMath::Abs(CurrentLaneX - TargetY) < 1.0f)
	{
		CurrentLane = TargetLane;
		CurrentLaneX = TargetY;
		// Snap to exact position when reached
		CurrentLocation.Y = TargetY;
		SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

bool ARabbitCharacter::CanInitiateLaneChange() const
{
	// If we're not moving, we can always initiate
	if (CurrentLane == TargetLane)
	{
		return true;
	}

	// Calculate a dynamic buffer based on turn speed and responsiveness
	// Faster turn speeds allow for larger buffers (earlier input acceptance)
	// Default LaneTransitionSpeed is 10.0, Responsiveness is 1.0 -> Buffer = 50.0 units
	// Lane width is 200.0, so 50.0 is "mostly done" (75% there)
	float TargetY = GetLaneXPosition(TargetLane);
	float DistanceToTarget = FMath::Abs(CurrentLaneX - TargetY);
	
	float AllowedBuffer = (LaneTransitionSpeed / 10.0f) * LaneChangeResponsiveness * 50.0f;
	
	return DistanceToTarget <= AllowedBuffer;
}

float ARabbitCharacter::GetLaneXPosition(ELanePosition Lane) const
{
	// Use fixed coordinates for locked-in lane system
	switch (Lane)
	{
	case ELanePosition::Left:
		return LANE_LEFT_Y;
	case ELanePosition::Center:
		return LANE_CENTER_Y;
	case ELanePosition::Right:
		return LANE_RIGHT_Y;
	default:
		return LANE_CENTER_Y;
	}
}

void ARabbitCharacter::OnMoveLeft(const FInputActionValue& Value)
{
	if (!bAutopilotActive)
	{
		MoveLeft();
	}
}

void ARabbitCharacter::OnMoveRight(const FInputActionValue& Value)
{
	if (!bAutopilotActive)
	{
		MoveRight();
	}
}

void ARabbitCharacter::OnJump(const FInputActionValue& Value)
{
	if (!bAutopilotActive)
	{
		// Use the jump component to perform jump (handles multi-jump logic internally)
		if (JumpComponent)
		{
			JumpComponent->PerformJump();
		}
		else
		{
			// Fallback to standard jump (only works on ground)
			if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
			{
				Jump();
			}
		}
	}
}

void ARabbitCharacter::OnSlide(const FInputActionValue& Value)
{
	Slide();
}

void ARabbitCharacter::OnSlideReleased(const FInputActionValue& Value)
{
	StopSlide();
}

void ARabbitCharacter::OnPause(const FInputActionValue& Value)
{
	// Get HUD and toggle pause
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
			{
				HUD->TogglePause();
			}
		}
	}
}

void ARabbitCharacter::UpdateAnimationState()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	// Check if jumping (in air)
	if (GetCharacterMovement()->IsFalling())
	{
		AnimationState = ERabbitAnimationState::Jumping;
	}
	// Check if ducking/sliding
	else if (SlideComponent && SlideComponent->IsSliding())
	{
		AnimationState = ERabbitAnimationState::Ducking;
	}
	// Otherwise, always running
	else
	{
		AnimationState = ERabbitAnimationState::Running;
	}
}

void ARabbitCharacter::SetInvincible(bool bInvincible, float Duration)
{
	bIsInvincible = bInvincible;
	
	// Clear any existing timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
		
		// If setting invincible with a duration, set a timer to disable it
		if (bInvincible && Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(InvincibilityTimerHandle, this, &ARabbitCharacter::OnInvincibilityExpired, Duration, false);
			UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Invincibility enabled for %.2f seconds"), Duration);
		}
		else if (!bInvincible)
		{
			UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Invincibility disabled"));
		}
	}
}

void ARabbitCharacter::OnInvincibilityExpired()
{
	bIsInvincible = false;
	UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Invincibility expired"));
}

void ARabbitCharacter::SetSpeedMultiplier(float Multiplier)
{
	// Updates GAS SpeedMultiplier attribute
	// Multiplier is stored additively (e.g., 2.0x speed = 1.0 multiplier value)
	float AdditiveMultiplier = FMath::Clamp(Multiplier, 0.5f, 3.0f) - 1.0f;
	
	if (AttributeSet)
	{
		AttributeSet->SetSpeedMultiplier(AdditiveMultiplier);
		ForwardSpeed = AttributeSet->GetCurrentSpeed();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RabbitCharacter: Cannot set speed multiplier - AttributeSet is null"));
		return;
	}
	
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = ForwardSpeed;
	}
	
	if (RabbitMovementComponent)
	{
		RabbitMovementComponent->SetForwardSpeed(ForwardSpeed);
	}
	
	UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Speed multiplier set to %.2f (ForwardSpeed: %.2f)"), Multiplier, ForwardSpeed);
}

void ARabbitCharacter::SetForwardSpeed(float NewSpeed)
{
	ForwardSpeed = NewSpeed;
	
	// Update movement components to match
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = ForwardSpeed;
	}
	
	if (RabbitMovementComponent)
	{
		RabbitMovementComponent->SetForwardSpeed(ForwardSpeed);
	}
	
	UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Forward speed set to %.2f"), ForwardSpeed);
}

void ARabbitCharacter::SetAutopilot(bool bEnabled)
{
	bAutopilotActive = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Autopilot %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void ARabbitCharacter::UpdateAutopilot(float DeltaTime)
{
	if (!bAutopilotActive)
	{
		return;
	}

	const float PowerUpLookAhead = 800.0f;
	const float HighObstacleLookAhead = 800.0f;
	const float FullObstacleLookAhead = 800.0f;
	const float LandingCheckDistance = 800.0f; // Check where we'll land when jumping

	// Check for power-ups in adjacent lanes - prioritize collecting them
	ELanePosition PowerUpLane = ELanePosition::Center;
	bool bFoundPowerUp = false;
	
	// Check all lanes for power-ups
	for (int32 LaneIdx = 0; LaneIdx < 3; ++LaneIdx)
	{
		ELanePosition CheckLane = static_cast<ELanePosition>(LaneIdx);
		if (HasPowerUpAhead(CheckLane, PowerUpLookAhead))
		{
			// Only change lanes if it's not the current lane and the lane is safe
			if (CheckLane != CurrentLane && IsLaneSafe(CheckLane, 800.0f))
			{
				PowerUpLane = CheckLane;
				bFoundPowerUp = true;
				UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot found power-up in %s lane, moving to collect"), 
					CheckLane == ELanePosition::Left ? TEXT("Left") : 
					CheckLane == ELanePosition::Center ? TEXT("Center") : TEXT("Right"));
				break;
			}
		}
	}

	// If we found a power-up in a safe lane, move to it
	if (bFoundPowerUp && PowerUpLane != CurrentLane && PowerUpLane != TargetLane)
	{
		TargetLane = PowerUpLane;
		return; // Prioritize power-up collection
	}

	// Check if we're currently jumping - if so, check where we'll land
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling())
	{
		// Check if we'll land near an obstacle in our target lane (the lane we're moving to)
		ELanePosition LaneToCheck = (TargetLane != CurrentLane) ? TargetLane : CurrentLane;
		if (HasObstacleTypeAhead(LaneToCheck, EObstacleType::Full, LandingCheckDistance) ||
			HasObstacleTypeAhead(LaneToCheck, EObstacleType::High, LandingCheckDistance))
		{
			// Find a safe lane to land in
			ELanePosition SafeLane = GetSafestLane();
			if (SafeLane != CurrentLane && SafeLane != TargetLane)
			{
				TargetLane = SafeLane;
				UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot changing lanes in air to avoid landing on obstacle"));
			}
		}
	}

	// Check for high obstacles - first check target lane if we're changing lanes
	if (TargetLane != CurrentLane && HasObstacleTypeAhead(TargetLane, EObstacleType::High, HighObstacleLookAhead))
	{
		// Obstacle in target lane - let current move complete, then dodge
		// Check if we're very close to target (within completion threshold)
		float TargetY = GetLaneXPosition(TargetLane);
		float DistanceToTarget = FMath::Abs(CurrentLaneX - TargetY);
		const float CompletionThreshold = 1.0f; // Same threshold used in UpdateLanePosition
		
		if (DistanceToTarget <= CompletionThreshold)
		{
			// We're at or very close to target - can change immediately (will complete this frame)
			ELanePosition SafeLane = GetSafestLane();
			if (SafeLane != CurrentLane && SafeLane != TargetLane)
			{
				TargetLane = SafeLane;
				UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot completing current move and dodging high obstacle"));
			}
		}
		else
		{
			// We're still moving - let it complete, then dodge on next frame
			// Don't change TargetLane yet, let the current move finish
			// The next frame when CurrentLane == TargetLane, we'll detect the obstacle and dodge
			UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot will complete current lane change, then dodge high obstacle"));
		}
	}
	// Then check current lane
	else if (HasObstacleTypeAhead(CurrentLane, EObstacleType::High, HighObstacleLookAhead))
	{
		// Obstacle is in current lane - try to find a safe lane to change to
		ELanePosition SafeLane = GetSafestLane();
		if (SafeLane != CurrentLane && SafeLane != TargetLane) // Can change lanes
		{
			TargetLane = SafeLane;
			UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot changing lanes to avoid high obstacle"));
		}
		else if (JumpComponent && !JumpComponent->IsJumping())
		{
			// No safe lane, jump over it
			if (GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround())
			{
				JumpComponent->PerformJump();
				UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot jumping over high obstacle"));
			}
		}
	}

	// Check for full obstacles - must change lanes (no jumping/sliding)
	// First, always check target lane if we're changing lanes (to catch obstacles in the lane we're moving to)
	if (TargetLane != CurrentLane && HasObstacleTypeAhead(TargetLane, EObstacleType::Full, FullObstacleLookAhead))
	{
		// Obstacle in target lane - let current move complete, then dodge
		// Check if we're very close to target (within completion threshold)
		float TargetY = GetLaneXPosition(TargetLane);
		float DistanceToTarget = FMath::Abs(CurrentLaneX - TargetY);
		const float CompletionThreshold = 1.0f; // Same threshold used in UpdateLanePosition
		
		if (DistanceToTarget <= CompletionThreshold)
		{
			// We're at or very close to target - can change immediately (will complete this frame)
			ELanePosition SafeLane = GetSafestLane();
			if (SafeLane != CurrentLane && SafeLane != TargetLane)
			{
				TargetLane = SafeLane;
				UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot completing current move and dodging full obstacle"));
			}
		}
		else
		{
			// We're still moving - let it complete, then dodge on next frame
			// Don't change TargetLane yet, let the current move finish
			// The next frame when CurrentLane == TargetLane, we'll detect the obstacle and dodge
			UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot will complete current lane change, then dodge full obstacle"));
		}
	}
	// Then check current lane
	else if (HasObstacleTypeAhead(CurrentLane, EObstacleType::Full, FullObstacleLookAhead))
	{
		// Obstacle is in current lane - change lanes
		ELanePosition SafeLane = GetSafestLane();
		if (SafeLane != CurrentLane && SafeLane != TargetLane)
		{
			TargetLane = SafeLane;
			UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot changing lanes to avoid full obstacle"));
		}
	}

	// Check for low obstacles - slide under
	if (HasObstacleTypeAhead(CurrentLane, EObstacleType::Low, 150.0f))
	{
		if (SlideComponent && !SlideComponent->IsSliding())
		{
			SlideComponent->StartSlide();
			UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot sliding under low obstacle"));
		}
	}

	// General lane safety check - move to safest lane if current is unsafe
	ELanePosition SafestLane = GetSafestLane();
	if (SafestLane != CurrentLane && SafestLane != TargetLane)
	{
		TargetLane = SafestLane;
		UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitCharacter: Autopilot moving to %s lane for safety"), 
			SafestLane == ELanePosition::Left ? TEXT("Left") : 
			SafestLane == ELanePosition::Center ? TEXT("Center") : TEXT("Right"));
	}
}

bool ARabbitCharacter::IsLaneSafe(ELanePosition Lane, float LookAheadDistance) const
{
	FVector PlayerLocation = GetActorLocation();
	float LaneY = GetLaneXPosition(Lane); // Note: GetLaneXPosition returns Y coordinate
	
	// Check position at the lane
	FVector CheckLocation = FVector(PlayerLocation.X + LookAheadDistance, LaneY, PlayerLocation.Z);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	UWorld* World = GetWorld();
	if (!World)
	{
		return true; // Assume safe if we can't check
	}

	// Use a box trace to check for obstacles in the lane
	FVector BoxExtent(50.0f, 50.0f, 100.0f);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
	
	if (World->SweepSingleByChannel(HitResult, PlayerLocation, CheckLocation, FQuat::Identity, ECC_WorldDynamic, BoxShape, QueryParams))
	{
		// Check if it's an obstacle
		if (Cast<AObstacle>(HitResult.GetActor()))
		{
			return false; // Lane has obstacle
		}
	}

	return true; // Lane appears safe
}

ELanePosition ARabbitCharacter::GetSafestLane() const
{
	// Only check within 800 units for autopilot
	const float AutopilotLookAhead = 800.0f;
	
	// Check all three lanes and return the safest one
	// Prefer current lane if it's safe
	if (IsLaneSafe(CurrentLane, AutopilotLookAhead))
	{
		return CurrentLane;
	}

	// Check adjacent lanes
	ELanePosition AdjacentLanes[2];
	if (CurrentLane == ELanePosition::Left)
	{
		AdjacentLanes[0] = ELanePosition::Center;
		AdjacentLanes[1] = ELanePosition::Right;
	}
	else if (CurrentLane == ELanePosition::Center)
	{
		AdjacentLanes[0] = ELanePosition::Left;
		AdjacentLanes[1] = ELanePosition::Right;
	}
	else // Right
	{
		AdjacentLanes[0] = ELanePosition::Center;
		AdjacentLanes[1] = ELanePosition::Left;
	}

	// Prefer center lane if safe
	for (int32 i = 0; i < 2; ++i)
	{
		if (IsLaneSafe(AdjacentLanes[i], AutopilotLookAhead))
		{
			return AdjacentLanes[i];
		}
	}

	// If no lane is safe, return current lane (player will have to deal with it)
	return CurrentLane;
}

bool ARabbitCharacter::HasPowerUpAhead(ELanePosition Lane, float LookAheadDistance) const
{
	FVector PlayerLocation = GetActorLocation();
	float LaneY = GetLaneXPosition(Lane);
	
	// Check position at the lane
	FVector CheckLocation = FVector(PlayerLocation.X + LookAheadDistance, LaneY, PlayerLocation.Z);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Use a box trace to check for power-ups in the lane
	FVector BoxExtent(50.0f, 50.0f, 100.0f);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
	
	if (World->SweepSingleByChannel(HitResult, PlayerLocation, CheckLocation, FQuat::Identity, ECC_WorldDynamic, BoxShape, QueryParams))
	{
		// Check if it's a power-up
		if (APowerUp* PowerUp = Cast<APowerUp>(HitResult.GetActor()))
		{
			return true;
		}
	}

	return false;
}

bool ARabbitCharacter::HasObstacleTypeAhead(ELanePosition Lane, EObstacleType ObstacleType, float LookAheadDistance) const
{
	FVector PlayerLocation = GetActorLocation();
	float LaneY = GetLaneXPosition(Lane);
	
	// Check position at the lane
	FVector CheckLocation = FVector(PlayerLocation.X + LookAheadDistance, LaneY, PlayerLocation.Z);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Use a box trace to check for obstacles in the lane
	FVector BoxExtent(50.0f, 50.0f, 100.0f);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
	
	if (World->SweepSingleByChannel(HitResult, PlayerLocation, CheckLocation, FQuat::Identity, ECC_WorldDynamic, BoxShape, QueryParams))
	{
		// Check if it's an obstacle of the specified type
		if (AObstacle* Obstacle = Cast<AObstacle>(HitResult.GetActor()))
		{
			return Obstacle->GetObstacleType() == ObstacleType;
		}
	}

	return false;
}

void ARabbitCharacter::EnableRagdollDeath(const FVector& LaunchVelocity)
{
	// Disable autopilot if active
	if (bAutopilotActive)
	{
		SetAutopilot(false);
	}
	
	// Disable character movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->DisableMovement();
		MovementComp->SetMovementMode(MOVE_None);
		MovementComp->StopMovementImmediately();
	}
	
	// Get the mesh component (skeletal mesh for ragdoll)
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		// Enable physics simulation on all bodies
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		// Set all bodies to simulate physics
		MeshComp->SetAllBodiesSimulatePhysics(true);
		MeshComp->SetAllBodiesPhysicsBlendWeight(1.0f);
		
		// Apply launch velocity to the root body
		MeshComp->SetPhysicsLinearVelocity(LaunchVelocity);
		
		// Add some angular velocity for spin
		FVector AngularVelocity = FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f));
		MeshComp->SetPhysicsAngularVelocityInRadians(AngularVelocity);
		
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Ragdoll enabled, launched with velocity (%.2f, %.2f, %.2f)"), 
			LaunchVelocity.X, LaunchVelocity.Y, LaunchVelocity.Z);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RabbitCharacter: No skeletal mesh component found for ragdoll"));
	}
	
	// Disable tick on movement components
	if (RabbitMovementComponent)
	{
		RabbitMovementComponent->SetComponentTickEnabled(false);
	}
	if (JumpComponent)
	{
		JumpComponent->SetComponentTickEnabled(false);
	}
	if (SlideComponent)
	{
		SlideComponent->SetComponentTickEnabled(false);
	}
	
	// Disable character's tick
	SetActorTickEnabled(false);
}

void ARabbitCharacter::ResetRagdollState()
{
	// Get the mesh component
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		// Disable physics simulation
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetAllBodiesSimulatePhysics(false);
		MeshComp->SetAllBodiesPhysicsBlendWeight(0.0f);
		
		// Reset mesh relative transform in case ragdoll affected it
		// Get the default relative transform from the blueprint/CDO to preserve the mesh's original position
		ARabbitCharacter* CDO = GetClass()->GetDefaultObject<ARabbitCharacter>();
		if (CDO && CDO->GetMesh())
		{
			// Use the blueprint's default relative transform
			FVector DefaultRelativeLocation = CDO->GetMesh()->GetRelativeLocation();
			FRotator DefaultRelativeRotation = CDO->GetMesh()->GetRelativeRotation();
			FVector DefaultRelativeScale = CDO->GetMesh()->GetRelativeScale3D();
			
			MeshComp->SetRelativeLocationAndRotation(DefaultRelativeLocation, DefaultRelativeRotation);
			MeshComp->SetRelativeScale3D(DefaultRelativeScale);
			
			UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Reset mesh to blueprint default relative transform - Location (%.2f, %.2f, %.2f)"), 
				DefaultRelativeLocation.X, DefaultRelativeLocation.Y, DefaultRelativeLocation.Z);
		}
		else
		{
			// Fallback to zero if CDO not available
			MeshComp->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
			MeshComp->SetRelativeScale3D(FVector::OneVector);
		}
		
		// Reset physics velocities
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
		
		// Force update the component transform
		MeshComp->UpdateComponentToWorld();
		
		// Reset collision settings to default character settings
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionObjectType(ECC_Pawn);
		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Ragdoll state reset - physics disabled"));
	}
	
	// Re-enable character movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
		MovementComp->SetActive(true);
		// Reset velocity
		MovementComp->Velocity = FVector::ZeroVector;
		MovementComp->UpdateComponentVelocity();
	}
	
	// Re-enable tick on movement components
	if (RabbitMovementComponent)
	{
		RabbitMovementComponent->SetComponentTickEnabled(true);
	}
	if (JumpComponent)
	{
		JumpComponent->SetComponentTickEnabled(true);
	}
	if (SlideComponent)
	{
		SlideComponent->SetComponentTickEnabled(true);
	}
	
	// Re-enable character's tick
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
}

void ARabbitCharacter::SetMaxJumpCount(int32 Count)
{
	if (JumpComponent)
	{
		JumpComponent->SetMaxJumpCount(Count);
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Set max jump count to %d"), Count);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RabbitCharacter: JumpComponent is null! Cannot set max jump count."));
	}
}

void ARabbitCharacter::SetCapsuleSize(float HalfHeight, float Radius)
{
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCapsuleHalfHeight(HalfHeight);
		CapsuleComp->SetCapsuleRadius(Radius);
		UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Set capsule size - HalfHeight: %.2f, Radius: %.2f"), HalfHeight, Radius);
	}
}

void ARabbitCharacter::SetNeverNeedsCrouch(bool bNeverNeeds)
{
	bNeverNeedsCrouch = bNeverNeeds;
	UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Set never needs crouch to %d"), bNeverNeeds ? 1 : 0);
}

void ARabbitCharacter::SetCanBreakObstacles(bool bCanBreak)
{
	bCanBreakObstacles = bCanBreak;
	UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Set can break obstacles to %d"), bCanBreak ? 1 : 0);
}

// GAS Interface Implementation
UAbilitySystemComponent* ARabbitCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ARabbitCharacter::GetCurrentSpeed() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetCurrentSpeed();
	}
	return BaseForwardSpeed;
}

float ARabbitCharacter::GetCurrentJumpHeight() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetCurrentJumpHeight();
	}
	return 300.0f; // Default jump height
}

int32 ARabbitCharacter::GetCurrentMaxJumpCount() const
{
	if (AttributeSet)
	{
		return FMath::RoundToInt(AttributeSet->GetCurrentMaxJumpCount());
	}
	return 1;
}

int32 ARabbitCharacter::GetCurrentLives() const
{
	if (AttributeSet)
	{
		return FMath::RoundToInt(AttributeSet->GetCurrentLives());
	}
	return 3;
}

float ARabbitCharacter::GetSpeedMultiplier() const
{
	if (AttributeSet)
	{
		return 1.0f + AttributeSet->GetSpeedMultiplier(); // Convert additive to multiplier
	}
	return 1.0f;
}

float ARabbitCharacter::GetCoinMultiplier() const
{
	if (AttributeSet)
	{
		return 1.0f + AttributeSet->GetCoinMultiplier(); // Convert additive to multiplier
	}
	return 1.0f;
}

float ARabbitCharacter::GetScoreMultiplier() const
{
	if (AttributeSet)
	{
		return 1.0f + AttributeSet->GetScoreMultiplier(); // Convert additive to multiplier
	}
	return 1.0f;
}

bool ARabbitCharacter::IsMagnetActiveFromGAS() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetMagnetActive() > 0.5f;
	}
	return false;
}

bool ARabbitCharacter::IsAutopilotActiveFromGAS() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetAutopilotActive() > 0.5f;
	}
	return false;
}

bool ARabbitCharacter::IsInvincibilityActiveFromGAS() const
{
	if (AttributeSet)
	{
		return AttributeSet->GetInvincibilityActive() > 0.5f;
	}
	return false;
}

void ARabbitCharacter::ResetGASEffects()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("RabbitCharacter: Cannot reset GAS effects - AbilitySystemComponent or AttributeSet is null"));
		return;
	}

	// Remove all active GameplayEffects (this clears all multipliers and temporary effects)
	FGameplayEffectQuery Query;
	Query.EffectSource = nullptr; // Remove all effects
	AbilitySystemComponent->RemoveActiveEffects(Query);
	
	// Reset all multiplier attributes to 0 (they're additive, so 0 = no multiplier)
	AttributeSet->SetSpeedMultiplier(0.0f);
	AttributeSet->SetJumpHeightMultiplier(0.0f);
	AttributeSet->SetCoinMultiplier(0.0f);
	AttributeSet->SetScoreMultiplier(0.0f);
	AttributeSet->SetLaneTransitionSpeedMultiplier(0.0f);
	AttributeSet->SetMultiJumpHeightMultiplier(0.0f);
	AttributeSet->SetGravityScaleMultiplier(0.0f);
	
	// Reset boolean-like attributes (magnet, autopilot, invincibility)
	AttributeSet->SetMagnetActive(0.0f);
	AttributeSet->SetAutopilotActive(0.0f);
	AttributeSet->SetInvincibilityActive(0.0f);
	
	// Reset all base attributes to their default values (from character properties)
	// These will be overridden by class perks in StartGame(), but we reset to defaults first
	AttributeSet->SetBaseSpeed(BaseForwardSpeed);
	AttributeSet->SetBaseJumpHeight(300.0f);
	AttributeSet->SetBaseMultiJumpHeight(200.0f);
	AttributeSet->SetBaseGravityScale(1.0f);
	AttributeSet->SetBaseLaneTransitionSpeed(BaseLaneTransitionSpeed);
	AttributeSet->SetBaseMaxJumpCount(1.0f);
	// BaseLives is set separately in StartGame() based on StartingLives
	
	UE_LOG(LogTemp, Log, TEXT("RabbitCharacter: Reset all GAS effects, multipliers, and base attributes to defaults"));
}

void ARabbitCharacter::ApplyEffectByTag(FGameplayTag Tag, float Value)
{
	if (!AbilitySystemComponent || !AttributeSet) return;

	// Identify which attribute to modify based on the tag
	if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("SpeedMultiplier"))))
	{
		AttributeSet->SetSpeedMultiplier(AttributeSet->GetSpeedMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("JumpHeightMultiplier"))))
	{
		AttributeSet->SetJumpHeightMultiplier(AttributeSet->GetJumpHeightMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("MultiJumpHeightMultiplier"))))
	{
		AttributeSet->SetMultiJumpHeightMultiplier(AttributeSet->GetMultiJumpHeightMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("GravityScaleMultiplier"))))
	{
		AttributeSet->SetGravityScaleMultiplier(AttributeSet->GetGravityScaleMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("LaneTransitionSpeedMultiplier"))))
	{
		AttributeSet->SetLaneTransitionSpeedMultiplier(AttributeSet->GetLaneTransitionSpeedMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("BaseMaxJumpCount"))))
	{
		AttributeSet->SetBaseMaxJumpCount(AttributeSet->GetBaseMaxJumpCount() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("BaseLives"))))
	{
		AttributeSet->SetBaseLives(AttributeSet->GetBaseLives() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("CoinMultiplier"))))
	{
		AttributeSet->SetCoinMultiplier(AttributeSet->GetCoinMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("ScoreMultiplier"))))
	{
		AttributeSet->SetScoreMultiplier(AttributeSet->GetScoreMultiplier() + Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("MagnetActive"))))
	{
		AttributeSet->SetMagnetActive(Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("AutopilotActive"))))
	{
		AttributeSet->SetAutopilotActive(Value);
	}
	else if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("InvincibilityActive"))))
	{
		AttributeSet->SetInvincibilityActive(Value);
	}
}

