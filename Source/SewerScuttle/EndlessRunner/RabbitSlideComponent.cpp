// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitSlideComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

URabbitSlideComponent::URabbitSlideComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URabbitSlideComponent::BeginPlay()
{
	Super::BeginPlay();

	// Store original capsule height
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		OriginalCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
		TargetCapsuleHalfHeight = OriginalCapsuleHalfHeight;
	}
}

void URabbitSlideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update cooldown
	if (CooldownRemaining > 0.0f)
	{
		CooldownRemaining -= DeltaTime;
	}

	// Update slide state
	if (bIsSliding)
	{
		UpdateSlide(DeltaTime);
	}

	// Smoothly transition capsule height
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		float CurrentHeight = Capsule->GetUnscaledCapsuleHalfHeight();
		float NewHeight = FMath::FInterpTo(CurrentHeight, TargetCapsuleHalfHeight, DeltaTime, SlideTransitionSpeed);
		Capsule->SetCapsuleHalfHeight(NewHeight);
	}
}

void URabbitSlideComponent::StartSlide()
{
	// Check cooldown
	if (CooldownRemaining > 0.0f)
	{
		return;
	}

	// Get character and movement component
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

	// If in air, perform stomp instead of sliding
	if (!MovementComp->IsMovingOnGround())
	{
		PerformStomp();
		return;
	}

	// Start sliding (on ground)
	bIsSliding = true;
	TargetCapsuleHalfHeight = SlideHeight;

	if (SlideDuration > 0.0f)
	{
		SlideTimeRemaining = SlideDuration;
	}
	else
	{
		SlideTimeRemaining = -1.0f; // Hold to slide
	}
}

void URabbitSlideComponent::StopSlide()
{
	if (!bIsSliding)
	{
		return;
	}

	bIsSliding = false;
	TargetCapsuleHalfHeight = OriginalCapsuleHalfHeight;
	CooldownRemaining = SlideCooldown;
}

void URabbitSlideComponent::PerformStomp()
{
	// Check cooldown
	if (CooldownRemaining > 0.0f)
	{
		return;
	}

	// Get character and movement component
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

	// Only perform stomp if in air
	if (!MovementComp->IsMovingOnGround())
	{
		// Apply instant downward velocity
		MovementComp->Velocity.Z = StompVelocity;
		
		// Set cooldown to prevent spam
		CooldownRemaining = SlideCooldown;
	}
}

void URabbitSlideComponent::UpdateSlide(float DeltaTime)
{
	// If duration is set, count down
	if (SlideTimeRemaining > 0.0f)
	{
		SlideTimeRemaining -= DeltaTime;
		if (SlideTimeRemaining <= 0.0f)
		{
			StopSlide();
		}
	}
	// Otherwise, check if still on ground
	else if (SlideTimeRemaining < 0.0f)
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
			if (MovementComp && !MovementComp->IsMovingOnGround())
			{
				StopSlide();
			}
		}
	}
}

UCapsuleComponent* URabbitSlideComponent::GetCapsuleComponent() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		return Character->GetCapsuleComponent();
	}
	return nullptr;
}


