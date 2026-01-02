// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FBossRewardData;

DECLARE_DELEGATE_OneParam(FOnRewardSelected, FString);

/**
 * Widget for selecting a reward after defeating a boss
 */
class SEWERSCUTTLE_API SBossRewardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBossRewardWidget)
	{}
		SLATE_EVENT(FOnRewardSelected, OnRewardSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update reward options */
	void UpdateRewards(const TArray<FBossRewardData>& Rewards);

	/** Delegate for reward selection */
	FOnRewardSelected OnRewardSelected;

private:
	/** Handle reward button click */
	FReply OnRewardButtonClicked(const FString& RewardId);

	/** Container for reward cards */
	TSharedPtr<class SHorizontalBox> RewardsContainer;
};

