// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitMovementComponent.h"

URabbitMovementComponent::URabbitMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxWalkSpeed = ForwardSpeed;
	MaxFlySpeed = ForwardSpeed;
	MaxCustomMovementSpeed = ForwardSpeed;
}

void URabbitMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update max speed to match forward speed
	MaxWalkSpeed = ForwardSpeed;
	MaxCustomMovementSpeed = ForwardSpeed;
}

void URabbitMovementComponent::SetForwardSpeed(float NewSpeed)
{
	ForwardSpeed = FMath::Clamp(NewSpeed, 100.0f, MaxForwardSpeed);
	MaxWalkSpeed = ForwardSpeed;
	MaxCustomMovementSpeed = ForwardSpeed;
}

