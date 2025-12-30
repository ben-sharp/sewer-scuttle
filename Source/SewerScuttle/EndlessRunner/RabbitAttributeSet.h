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
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_BaseSpeed)
	FGameplayAttributeData BaseSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseSpeed)

	// Current Speed (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_CurrentSpeed)
	FGameplayAttributeData CurrentSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentSpeed)

	// Speed Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_SpeedMultiplier)
	FGameplayAttributeData SpeedMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, SpeedMultiplier)

	// Base Jump Height
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_BaseJumpHeight)
	FGameplayAttributeData BaseJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseJumpHeight)

	// Current Jump Height (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_CurrentJumpHeight)
	FGameplayAttributeData CurrentJumpHeight;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentJumpHeight)

	// Jump Height Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_JumpHeightMultiplier)
	FGameplayAttributeData JumpHeightMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, JumpHeightMultiplier)

	// Base Lane Transition Speed
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_BaseLaneTransitionSpeed)
	FGameplayAttributeData BaseLaneTransitionSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseLaneTransitionSpeed)

	// Current Lane Transition Speed (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_CurrentLaneTransitionSpeed)
	FGameplayAttributeData CurrentLaneTransitionSpeed;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentLaneTransitionSpeed)

	// Lane Transition Speed Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_LaneTransitionSpeedMultiplier)
	FGameplayAttributeData LaneTransitionSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, LaneTransitionSpeedMultiplier)

	// Combat Stats
	// Base Lives
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_BaseLives)
	FGameplayAttributeData BaseLives;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseLives)

	// Current Lives (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CurrentLives)
	FGameplayAttributeData CurrentLives;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentLives)

	// Base Max Jump Count
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_BaseMaxJumpCount)
	FGameplayAttributeData BaseMaxJumpCount;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, BaseMaxJumpCount)

	// Current Max Jump Count (calculated from base + modifiers)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CurrentMaxJumpCount)
	FGameplayAttributeData CurrentMaxJumpCount;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CurrentMaxJumpCount)

	// Utility Stats
	// Coin Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Utility", ReplicatedUsing = OnRep_CoinMultiplier)
	FGameplayAttributeData CoinMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, CoinMultiplier)

	// Score Multiplier (temporary, stacks additively)
	UPROPERTY(BlueprintReadOnly, Category = "Utility", ReplicatedUsing = OnRep_ScoreMultiplier)
	FGameplayAttributeData ScoreMultiplier;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, ScoreMultiplier)

	// Special Stats (boolean flags stored as floats: 0.0 = false, 1.0 = true)
	// Magnet Active
	UPROPERTY(BlueprintReadOnly, Category = "Special", ReplicatedUsing = OnRep_MagnetActive)
	FGameplayAttributeData MagnetActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, MagnetActive)

	// Autopilot Active
	UPROPERTY(BlueprintReadOnly, Category = "Special", ReplicatedUsing = OnRep_AutopilotActive)
	FGameplayAttributeData AutopilotActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, AutopilotActive)

	// Invincibility Active
	UPROPERTY(BlueprintReadOnly, Category = "Special", ReplicatedUsing = OnRep_InvincibilityActive)
	FGameplayAttributeData InvincibilityActive;
	ATTRIBUTE_ACCESSORS(URabbitAttributeSet, InvincibilityActive)

protected:
	// Replication functions
	UFUNCTION()
	virtual void OnRep_BaseSpeed(const FGameplayAttributeData& OldBaseSpeed);
	UFUNCTION()
	virtual void OnRep_CurrentSpeed(const FGameplayAttributeData& OldCurrentSpeed);
	UFUNCTION()
	virtual void OnRep_SpeedMultiplier(const FGameplayAttributeData& OldSpeedMultiplier);
	UFUNCTION()
	virtual void OnRep_BaseJumpHeight(const FGameplayAttributeData& OldBaseJumpHeight);
	UFUNCTION()
	virtual void OnRep_CurrentJumpHeight(const FGameplayAttributeData& OldCurrentJumpHeight);
	UFUNCTION()
	virtual void OnRep_JumpHeightMultiplier(const FGameplayAttributeData& OldJumpHeightMultiplier);
	UFUNCTION()
	virtual void OnRep_BaseLaneTransitionSpeed(const FGameplayAttributeData& OldBaseLaneTransitionSpeed);
	UFUNCTION()
	virtual void OnRep_CurrentLaneTransitionSpeed(const FGameplayAttributeData& OldCurrentLaneTransitionSpeed);
	UFUNCTION()
	virtual void OnRep_LaneTransitionSpeedMultiplier(const FGameplayAttributeData& OldLaneTransitionSpeedMultiplier);
	UFUNCTION()
	virtual void OnRep_BaseLives(const FGameplayAttributeData& OldBaseLives);
	UFUNCTION()
	virtual void OnRep_CurrentLives(const FGameplayAttributeData& OldCurrentLives);
	UFUNCTION()
	virtual void OnRep_BaseMaxJumpCount(const FGameplayAttributeData& OldBaseMaxJumpCount);
	UFUNCTION()
	virtual void OnRep_CurrentMaxJumpCount(const FGameplayAttributeData& OldCurrentMaxJumpCount);
	UFUNCTION()
	virtual void OnRep_CoinMultiplier(const FGameplayAttributeData& OldCoinMultiplier);
	UFUNCTION()
	virtual void OnRep_ScoreMultiplier(const FGameplayAttributeData& OldScoreMultiplier);
	UFUNCTION()
	virtual void OnRep_MagnetActive(const FGameplayAttributeData& OldMagnetActive);
	UFUNCTION()
	virtual void OnRep_AutopilotActive(const FGameplayAttributeData& OldAutopilotActive);
	UFUNCTION()
	virtual void OnRep_InvincibilityActive(const FGameplayAttributeData& OldInvincibilityActive);

	// Calculate current values from base + modifiers
	void UpdateCurrentSpeed();
	void UpdateCurrentJumpHeight();
	void UpdateCurrentLaneTransitionSpeed();
	void UpdateCurrentLives();
	void UpdateCurrentMaxJumpCount();

public:
	// Override to calculate current values when base or modifiers change
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
};




