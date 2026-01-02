// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffect.h"

URabbitAttributeSet::URabbitAttributeSet()
{
	// Initialize default values
	BaseSpeed.SetBaseValue(1000.0f);
	BaseSpeed.SetCurrentValue(1000.0f);
	
	CurrentSpeed.SetBaseValue(1000.0f);
	CurrentSpeed.SetCurrentValue(1000.0f);
	
	SpeedMultiplier.SetBaseValue(0.0f);
	SpeedMultiplier.SetCurrentValue(0.0f);
	
	BaseJumpHeight.SetBaseValue(300.0f);
	BaseJumpHeight.SetCurrentValue(300.0f);
	
	CurrentJumpHeight.SetBaseValue(300.0f);
	CurrentJumpHeight.SetCurrentValue(300.0f);
	
	JumpHeightMultiplier.SetBaseValue(0.0f);
	JumpHeightMultiplier.SetCurrentValue(0.0f);
	
	BaseLaneTransitionSpeed.SetBaseValue(10.0f);
	BaseLaneTransitionSpeed.SetCurrentValue(10.0f);
	
	CurrentLaneTransitionSpeed.SetBaseValue(10.0f);
	CurrentLaneTransitionSpeed.SetCurrentValue(10.0f);
	
	LaneTransitionSpeedMultiplier.SetBaseValue(0.0f);
	LaneTransitionSpeedMultiplier.SetCurrentValue(0.0f);
	
	BaseLives.SetBaseValue(3.0f);
	BaseLives.SetCurrentValue(3.0f);
	
	CurrentLives.SetBaseValue(3.0f);
	CurrentLives.SetCurrentValue(3.0f);
	
	BaseMaxJumpCount.SetBaseValue(1.0f);
	BaseMaxJumpCount.SetCurrentValue(1.0f);
	
	CurrentMaxJumpCount.SetBaseValue(1.0f);
	CurrentMaxJumpCount.SetCurrentValue(1.0f);
	
	CoinMultiplier.SetBaseValue(0.0f);
	CoinMultiplier.SetCurrentValue(0.0f);
	
	ScoreMultiplier.SetBaseValue(0.0f);
	ScoreMultiplier.SetCurrentValue(0.0f);
	
	MagnetActive.SetBaseValue(0.0f);
	MagnetActive.SetCurrentValue(0.0f);
	
	AutopilotActive.SetBaseValue(0.0f);
	AutopilotActive.SetCurrentValue(0.0f);
	
	InvincibilityActive.SetBaseValue(0.0f);
	InvincibilityActive.SetCurrentValue(0.0f);
}

void URabbitAttributeSet::UpdateCurrentSpeed()
{
	float NewCurrentSpeed = BaseSpeed.GetCurrentValue() * (1.0f + SpeedMultiplier.GetCurrentValue());
	CurrentSpeed.SetCurrentValue(NewCurrentSpeed);
}

void URabbitAttributeSet::UpdateCurrentJumpHeight()
{
	float NewCurrentJumpHeight = BaseJumpHeight.GetCurrentValue() * (1.0f + JumpHeightMultiplier.GetCurrentValue());
	CurrentJumpHeight.SetCurrentValue(NewCurrentJumpHeight);
}

void URabbitAttributeSet::UpdateCurrentLaneTransitionSpeed()
{
	float NewCurrentLaneTransitionSpeed = BaseLaneTransitionSpeed.GetCurrentValue() * (1.0f + LaneTransitionSpeedMultiplier.GetCurrentValue());
	CurrentLaneTransitionSpeed.SetCurrentValue(NewCurrentLaneTransitionSpeed);
}

void URabbitAttributeSet::UpdateCurrentLives()
{
	// Lives are typically just set directly, but we can calculate if needed
	CurrentLives.SetCurrentValue(BaseLives.GetCurrentValue());
}

void URabbitAttributeSet::UpdateCurrentMaxJumpCount()
{
	// Max jump count is typically just set directly, but we can calculate if needed
	CurrentMaxJumpCount.SetCurrentValue(BaseMaxJumpCount.GetCurrentValue());
}

void URabbitAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp values if needed
	if (Attribute == GetBaseSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 100.0f, 2000.0f);
	}
	else if (Attribute == GetBaseJumpHeightAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 100.0f, 1000.0f);
	}
	else if (Attribute == GetBaseLivesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 10.0f);
	}
	else if (Attribute == GetBaseMaxJumpCountAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void URabbitAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Update calculated values when base or multiplier attributes change
	if (Attribute == GetBaseSpeedAttribute() || Attribute == GetSpeedMultiplierAttribute())
	{
		UpdateCurrentSpeed();
	}
	else if (Attribute == GetBaseJumpHeightAttribute() || Attribute == GetJumpHeightMultiplierAttribute())
	{
		UpdateCurrentJumpHeight();
	}
	else if (Attribute == GetBaseLaneTransitionSpeedAttribute() || Attribute == GetLaneTransitionSpeedMultiplierAttribute())
	{
		UpdateCurrentLaneTransitionSpeed();
	}
	else if (Attribute == GetBaseLivesAttribute())
	{
		UpdateCurrentLives();
	}
	else if (Attribute == GetBaseMaxJumpCountAttribute())
	{
		UpdateCurrentMaxJumpCount();
	}
}
