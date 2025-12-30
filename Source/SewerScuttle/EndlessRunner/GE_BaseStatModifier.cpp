// Copyright Epic Games, Inc. All Rights Reserved.

#include "GE_BaseStatModifier.h"

UGE_BaseStatModifier::UGE_BaseStatModifier()
{
	// Set duration to infinite (permanent for the run)
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	
	// Set stacking to aggregate by source - each power-up instance creates a separate stack
	// This allows multiple power-up collections to add to the total
	// Note: StackingType is deprecated but still functional - will be updated when new API is available
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	StackingType = EGameplayEffectStackingType::AggregateBySource;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	// This will be configured in Blueprint or child classes
	// to specify which attribute to modify and by how much
}

