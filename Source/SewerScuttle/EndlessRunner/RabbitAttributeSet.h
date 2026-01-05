// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "RabbitAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute set for Rabbit character stats
 * Manages base stats, current stats, and temporary modifiers
 */
UCLASS()
class SEWERSCUTTLE_API URabbitAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	URabbitAttributeSet();

	// Movement Stats
	// Base Speed
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseSpeed)

	// Current Speed (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentSpeed)

	// Speed Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData SpeedMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, SpeedMultiplier)

	// Base Jump Height
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseJumpHeight)

	// Current Jump Height (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentJumpHeight)

	// Jump Height Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData JumpHeightMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, JumpHeightMultiplier)

	// Base Lane Transition Speed
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseLaneTransitionSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseLaneTransitionSpeed)

	// Current Lane Transition Speed (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentLaneTransitionSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentLaneTransitionSpeed)

	// Lane Transition Speed Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData LaneTransitionSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, LaneTransitionSpeedMultiplier)

	// Base Lane Change Responsiveness
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseLaneChangeResponsiveness;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseLaneChangeResponsiveness)

	// Current Lane Change Responsiveness (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentLaneChangeResponsiveness;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentLaneChangeResponsiveness)

	// Lane Change Responsiveness Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData LaneChangeResponsivenessMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, LaneChangeResponsivenessMultiplier)

	// Base Multi Jump Height
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseMultiJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseMultiJumpHeight)

	// Current Multi Jump Height
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentMultiJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentMultiJumpHeight)

	// Multi Jump Height Multiplier
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MultiJumpHeightMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, MultiJumpHeightMultiplier)

	// Base Gravity Scale (Weight)
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BaseGravityScale;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseGravityScale)

	// Current Gravity Scale
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CurrentGravityScale;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentGravityScale)

	// Gravity Scale Multiplier
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData GravityScaleMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, GravityScaleMultiplier)

	// Combat Stats
	// Base Lives
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData BaseLives;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseLives)

	// Current Lives (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData CurrentLives;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentLives)

	// Base Max Jump Count
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData BaseMaxJumpCount;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseMaxJumpCount)

	// Current Max Jump Count (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData CurrentMaxJumpCount;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentMaxJumpCount)

	// Utility Stats
	// Coin Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	FGameplayAttributeData CoinMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CoinMultiplier)

	// Score Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	FGameplayAttributeData ScoreMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, ScoreMultiplier)

	// Special Stats (boolean flags stored as floats: 0.0 = false, 1.0 = true)
	// Magnet Active
	UPROPERTY(BlueprintReadOnly, Category = "Special")
	FGameplayAttributeData MagnetActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, MagnetActive)

	// Autopilot Active
	UPROPERTY(BlueprintReadOnly, Category = "Special")
	FGameplayAttributeData AutopilotActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, AutopilotActive)

	// Invincibility Active
	UPROPERTY(BlueprintReadOnly, Category = "Special")
	FGameplayAttributeData InvincibilityActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, InvincibilityActive)

protected:
	// Calculate current values from base + modifiers
	void UpdateCurrentSpeed();
	void UpdateCurrentJumpHeight();
	void UpdateCurrentLaneTransitionSpeed();
	void UpdateCurrentLaneChangeResponsiveness();
	void UpdateCurrentMultiJumpHeight();
	void UpdateCurrentGravityScale();
	void UpdateCurrentLives();
	void UpdateCurrentMaxJumpCount();

public:
	// Override to calculate current values when base or modifiers change
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
};




















