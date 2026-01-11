// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "EndlessRunner/ReplayModels.h"

/** Delegate for back button */
DECLARE_DELEGATE(FOnBackClicked);

/** Delegate for watch replay button */
DECLARE_DELEGATE_OneParam(FOnWatchReplayClicked, int32 /* RunId */);

/** Delegate for class tab change */
DECLARE_DELEGATE_OneParam(FOnClassTabChanged, FString /* ClassName */);

/**
 * Slate widget for leaderboard display
 * Shows top scores and player's rank
 */
class SEWERSCUTTLE_API SLeaderboardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLeaderboardWidget)
	{}
		SLATE_EVENT(FOnBackClicked, OnBackClicked)
		SLATE_EVENT(FOnWatchReplayClicked, OnWatchReplayClicked)
		SLATE_EVENT(FOnClassTabChanged, OnClassTabChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update leaderboard data */
	void UpdateLeaderboard(const TArray<FLeaderboardEntryData>& Entries, int32 PlayerRank);

	/** Set loading state */
	void SetLoading(bool bLoading);

	FOnBackClicked OnBackClicked;
	FOnWatchReplayClicked OnWatchReplayClicked;
	FOnClassTabChanged OnClassTabChanged;

private:
	/** Current selected class tab */
	FString SelectedClass;

	/** Leaderboard entries container */
	TSharedPtr<class SVerticalBox> LeaderboardContainer;

	/** Main vertical stack */
	TSharedPtr<class SVerticalBox> MainStack;

	/** Player rank text */
	TSharedPtr<class STextBlock> PlayerRankText;

	/** Loading overlay */
	TSharedPtr<class SWidget> LoadingWidget;

	/** Create a class tab button */
	TSharedRef<class SWidget> CreateClassTab(const FString& ClassName, const FText& DisplayName);

	/** Handle tab click */
	FReply OnTabClicked(FString ClassName);

	/** Handle back button click */
	FReply OnBackButtonClicked();

	/** Rebuild the entire widget UI */
	void RebuildUI();
};

