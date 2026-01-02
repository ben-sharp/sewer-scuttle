// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitJumpComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RabbitCharacter.h"
#include "AbilitySystemComponent.h"

URabbitJumpComponent::URabbitJumpComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URabbitJumpComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URabbitJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update cooldown
	if (CooldownRemaining > 0.0f)
	{
		CooldownRemaining -= DeltaTime;
	}

	// Update jump state
	if (bIsJumping)
	{
		UpdateJump(DeltaTime);
	}
}

void URabbitJumpComponent::PerformJump()
{
	// Get character and movement component first
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Check if we can jump (either on ground or have jumps remaining for multi-jump)
	bool bOnGround = MovementComp->IsMovingOnGround();
	
	// If on ground, reset jump count and allow jump
	if (bOnGround)
	{
		CurrentJumpCount = 0;
		bIsJumping = false; // Reset jumping state when on ground
	}
	else
	{
		// In air - check if we have multi-jumps remaining
		if (CurrentJumpCount >= GetMaxJumpCount())
		{
			return;
		}
	}

	// Check cooldown (only for ground jumps to prevent spam)
	if (bOnGround && CooldownRemaining > 0.0f)
	{
		return;
	}

	// Don't check bIsJumping for multi-jump - we want to allow jumping in air

	// Perform jump - handle both ground and air jumps manually for full control
	FVector JumpVelocity = MovementComp->Velocity;
	
	if (bOnGround)
	{
		// On ground - jump velocity from base jump height
		float CurrentJumpHeight = GetJumpHeight();
		float BaseJumpVelocity = FMath::Sqrt(2.0f * CurrentJumpHeight * MovementComp->GetGravityZ() * -1.0f);
		JumpVelocity.Z = BaseJumpVelocity;
		CurrentJumpCount = 1; // First jump
		CooldownRemaining = JumpCooldown;
		UE_LOG(LogTemp, VeryVerbose, TEXT("RabbitJumpComponent: Ground jump (Velocity.Z=%.2f)"), JumpVelocity.Z);
	}
	else
	{
		// In air - jump velocity from multi jump height
		float CurrentMultiJumpHeight = GetMultiJumpHeight();
		float MultiJumpVelocity = FMath::Sqrt(2.0f * CurrentMultiJumpHeight * MovementComp->GetGravityZ() * -1.0f);
		JumpVelocity.Z = FMath::Max(JumpVelocity.Z, 0.0f) + MultiJumpVelocity;
		
		// Increment jump count for multi-jump
		CurrentJumpCount++;
		int32 CurrentMaxJumpCount = GetMaxJumpCount();
		UE_LOG(LogTemp, Warning, TEXT("RabbitJumpComponent: Multi-jump used (Count: %d/%d, Velocity.Z=%.2f)"), 
			CurrentJumpCount, CurrentMaxJumpCount, JumpVelocity.Z);
	}
	
	// Apply velocity and set movement mode
	MovementComp->Velocity = JumpVelocity;
	MovementComp->SetMovementMode(MOVE_Falling);
	
	// Set jumping state and timing
	bIsJumping = true;
	JumpTimeRemaining = JumpDuration;
}

void URabbitJumpComponent::UpdateJump(float DeltaTime)
{
	JumpTimeRemaining -= DeltaTime;

	// Check if jump is complete
	if (JumpTimeRemaining <= 0.0f)
	{
		bIsJumping = false;
		JumpTimeRemaining = 0.0f;
	}
	else
	{
		// Check if character landed
		UCharacterMovementComponent* MovementComp = GetCharacterMovement();
		if (MovementComp && MovementComp->IsMovingOnGround())
		{
			bIsJumping = false;
			JumpTimeRemaining = 0.0f;
			// Reset jump count when landing
			CurrentJumpCount = 0;
		}
	}
}

UCharacterMovementComponent* URabbitJumpComponent::GetCharacterMovement() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		return Character->GetCharacterMovement();
	}
	return nullptr;
}

float URabbitJumpComponent::GetJumpHeight() const
{
	// Try to get from GAS first
	ARabbitCharacter* RabbitChar = Cast<ARabbitCharacter>(GetOwner());
	if (RabbitChar && RabbitChar->GetAttributeSet())
	{
		return RabbitChar->GetAttributeSet()->GetCurrentJumpHeight();
	}
	// Fallback to local property
	return JumpHeight;
}

float URabbitJumpComponent::GetMultiJumpHeight() const
{
	// Try to get from GAS first
	ARabbitCharacter* RabbitChar = Cast<ARabbitCharacter>(GetOwner());
	if (RabbitChar && RabbitChar->GetAttributeSet())
	{
		return RabbitChar->GetAttributeSet()->GetCurrentMultiJumpHeight();
	}
	// Fallback to local property (reduced jump height for air jumps)
	return GetJumpHeight() * MultiJumpVelocityMultiplier;
}

void URabbitJumpComponent::SetMaxJumpCount(int32 Count)
{
	MaxJumpCount = Count;
	
	// Also update GAS if available
	ARabbitCharacter* RabbitChar = Cast<ARabbitCharacter>(GetOwner());
	if (RabbitChar && RabbitChar->GetAttributeSet())
	{
		RabbitChar->GetAttributeSet()->SetBaseMaxJumpCount(static_cast<float>(Count));
	}
}

int32 URabbitJumpComponent::GetMaxJumpCount() const
{
	// Try to get from GAS first
	ARabbitCharacter* RabbitChar = Cast<ARabbitCharacter>(GetOwner());
	if (RabbitChar && RabbitChar->GetAttributeSet())
	{
		return FMath::RoundToInt(RabbitChar->GetAttributeSet()->GetCurrentMaxJumpCount());
	}
	// Fallback to local property
	return MaxJumpCount;
}

