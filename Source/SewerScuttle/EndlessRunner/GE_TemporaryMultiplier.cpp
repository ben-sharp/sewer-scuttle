// Copyright Epic Games, Inc. All Rights Reserved.

#include "GE_TemporaryMultiplier.h"

UGE_TemporaryMultiplier::UGE_TemporaryMultiplier()
{
	// Set duration policy to HasDuration (temporary)
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	
	// Set stacking to aggregate by target - multiple instances will stack additively
	// Note: StackingType is deprecated but still functional - will be updated when new API is available
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	// Default duration will be set in Blueprint or child classes
	// This will be configured in Blueprint or child classes
	// to specify which multiplier attribute to modify and by how much
}

