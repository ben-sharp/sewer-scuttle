// Copyright Epic Games, Inc. All Rights Reserved.

#include "RabbitAttributeSet.h"
#include <type_traits>
#include "Net/UnrealNetwork.h"

URabbitAttributeSet::URabbitAttributeSet()
{
	// Initialize base values
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

void URabbitAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, BaseSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CurrentSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, SpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, BaseJumpHeight, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CurrentJumpHeight, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, JumpHeightMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, BaseLaneTransitionSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CurrentLaneTransitionSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, LaneTransitionSpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, BaseLives, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CurrentLives, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, BaseMaxJumpCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CurrentMaxJumpCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, CoinMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, ScoreMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, MagnetActive, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, AutopilotActive, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URabbitAttributeSet, InvincibilityActive, COND_None, REPNOTIFY_Always);
}

void URabbitAttributeSet::OnRep_BaseSpeed(const FGameplayAttributeData& OldBaseSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, BaseSpeed, OldBaseSpeed);
	UpdateCurrentSpeed();
}

void URabbitAttributeSet::OnRep_CurrentSpeed(const FGameplayAttributeData& OldCurrentSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CurrentSpeed, OldCurrentSpeed);
}

void URabbitAttributeSet::OnRep_SpeedMultiplier(const FGameplayAttributeData& OldSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, SpeedMultiplier, OldSpeedMultiplier);
	UpdateCurrentSpeed();
}

void URabbitAttributeSet::OnRep_BaseJumpHeight(const FGameplayAttributeData& OldBaseJumpHeight)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, BaseJumpHeight, OldBaseJumpHeight);
	UpdateCurrentJumpHeight();
}

void URabbitAttributeSet::OnRep_CurrentJumpHeight(const FGameplayAttributeData& OldCurrentJumpHeight)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CurrentJumpHeight, OldCurrentJumpHeight);
}

void URabbitAttributeSet::OnRep_JumpHeightMultiplier(const FGameplayAttributeData& OldJumpHeightMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, JumpHeightMultiplier, OldJumpHeightMultiplier);
	UpdateCurrentJumpHeight();
}

void URabbitAttributeSet::OnRep_BaseLaneTransitionSpeed(const FGameplayAttributeData& OldBaseLaneTransitionSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, BaseLaneTransitionSpeed, OldBaseLaneTransitionSpeed);
	UpdateCurrentLaneTransitionSpeed();
}

void URabbitAttributeSet::OnRep_CurrentLaneTransitionSpeed(const FGameplayAttributeData& OldCurrentLaneTransitionSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CurrentLaneTransitionSpeed, OldCurrentLaneTransitionSpeed);
}

void URabbitAttributeSet::OnRep_LaneTransitionSpeedMultiplier(const FGameplayAttributeData& OldLaneTransitionSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, LaneTransitionSpeedMultiplier, OldLaneTransitionSpeedMultiplier);
	UpdateCurrentLaneTransitionSpeed();
}

void URabbitAttributeSet::OnRep_BaseLives(const FGameplayAttributeData& OldBaseLives)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, BaseLives, OldBaseLives);
	UpdateCurrentLives();
}

void URabbitAttributeSet::OnRep_CurrentLives(const FGameplayAttributeData& OldCurrentLives)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CurrentLives, OldCurrentLives);
}

void URabbitAttributeSet::OnRep_BaseMaxJumpCount(const FGameplayAttributeData& OldBaseMaxJumpCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, BaseMaxJumpCount, OldBaseMaxJumpCount);
	UpdateCurrentMaxJumpCount();
}

void URabbitAttributeSet::OnRep_CurrentMaxJumpCount(const FGameplayAttributeData& OldCurrentMaxJumpCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CurrentMaxJumpCount, OldCurrentMaxJumpCount);
}

void URabbitAttributeSet::OnRep_CoinMultiplier(const FGameplayAttributeData& OldCoinMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, CoinMultiplier, OldCoinMultiplier);
}

void URabbitAttributeSet::OnRep_ScoreMultiplier(const FGameplayAttributeData& OldScoreMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, ScoreMultiplier, OldScoreMultiplier);
}

void URabbitAttributeSet::OnRep_MagnetActive(const FGameplayAttributeData& OldMagnetActive)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, MagnetActive, OldMagnetActive);
}

void URabbitAttributeSet::OnRep_AutopilotActive(const FGameplayAttributeData& OldAutopilotActive)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, AutopilotActive, OldAutopilotActive);
}

void URabbitAttributeSet::OnRep_InvincibilityActive(const FGameplayAttributeData& OldInvincibilityActive)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URabbitAttributeSet, InvincibilityActive, OldInvincibilityActive);
}

void URabbitAttributeSet::UpdateCurrentSpeed()
{
	float Base = BaseSpeed.GetCurrentValue();
	float Multiplier = SpeedMultiplier.GetCurrentValue();
	float Current = Base * (1.0f + Multiplier);
	CurrentSpeed.SetCurrentValue(Current);
}

void URabbitAttributeSet::UpdateCurrentJumpHeight()
{
	float Base = BaseJumpHeight.GetCurrentValue();
	float Multiplier = JumpHeightMultiplier.GetCurrentValue();
	float Current = Base * (1.0f + Multiplier);
	CurrentJumpHeight.SetCurrentValue(Current);
}

void URabbitAttributeSet::UpdateCurrentLaneTransitionSpeed()
{
	float Base = BaseLaneTransitionSpeed.GetCurrentValue();
	float Multiplier = LaneTransitionSpeedMultiplier.GetCurrentValue();
	float Current = Base * (1.0f + Multiplier);
	CurrentLaneTransitionSpeed.SetCurrentValue(Current);
}

void URabbitAttributeSet::UpdateCurrentLives()
{
	// Lives are additive (base + permanent modifications)
	// For now, just use base value (permanent modifications will be added via Gameplay Effects)
	CurrentLives.SetCurrentValue(BaseLives.GetCurrentValue());
}

void URabbitAttributeSet::UpdateCurrentMaxJumpCount()
{
	// Max jump count is additive (base + permanent modifications)
	// For now, just use base value (permanent modifications will be added via Gameplay Effects)
	CurrentMaxJumpCount.SetCurrentValue(BaseMaxJumpCount.GetCurrentValue());
}

void URabbitAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp values if needed
	if (Attribute == GetBaseSpeedAttribute() || Attribute == GetCurrentSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 100.0f, 2000.0f);
	}
	else if (Attribute == GetBaseJumpHeightAttribute() || Attribute == GetCurrentJumpHeightAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 100.0f, 1000.0f);
	}
	else if (Attribute == GetBaseLivesAttribute() || Attribute == GetCurrentLivesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 10.0f);
	}
	else if (Attribute == GetBaseMaxJumpCountAttribute() || Attribute == GetCurrentMaxJumpCountAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void URabbitAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Update calculated current values when base or multipliers change
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

