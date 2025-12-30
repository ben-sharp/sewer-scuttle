// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Slate widget for leaderboard display
 * Shows top scores and player's rank
 */
class SEWERSCUTTLE_API SLeaderboardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLeaderboardWidget)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update leaderboard data */
	void UpdateLeaderboard(const TArray<FString>& TopScores, int32 PlayerRank);

	/** Delegate for back button */
	DECLARE_DELEGATE(FOnBackClicked);
	FOnBackClicked OnBackClicked;

private:
	/** Leaderboard entries container */
	TSharedPtr<class SVerticalBox> LeaderboardContainer;

	/** Player rank text */
	TSharedPtr<class STextBlock> PlayerRankText;

	/** Handle back button click */
	FReply OnBackButtonClicked();
};

