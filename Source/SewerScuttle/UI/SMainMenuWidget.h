// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Slate widget for main menu
 * Provides buttons for Play, Shop, Leaderboard, and Settings
 */
class SEWERSCUTTLE_API SMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainMenuWidget)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Delegate for play button */
	DECLARE_DELEGATE(FOnPlayClicked);
	FOnPlayClicked OnPlayClicked;

	/** Delegate for shop button */
	DECLARE_DELEGATE(FOnShopClicked);
	FOnShopClicked OnShopClicked;

	/** Delegate for leaderboard button */
	DECLARE_DELEGATE(FOnLeaderboardClicked);
	FOnLeaderboardClicked OnLeaderboardClicked;

	/** Delegate for settings button */
	DECLARE_DELEGATE(FOnSettingsClicked);
	FOnSettingsClicked OnSettingsClicked;

private:
	/** Handle play button click */
	FReply OnPlayButtonClicked();

	/** Handle shop button click */
	FReply OnShopButtonClicked();

	/** Handle leaderboard button click */
	FReply OnLeaderboardButtonClicked();

	/** Handle settings button click */
	FReply OnSettingsButtonClicked();
};

