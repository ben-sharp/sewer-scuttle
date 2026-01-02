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
		SLATE_EVENT(FSimpleDelegate, OnPlayClicked)
		SLATE_EVENT(FSimpleDelegate, OnShopClicked)
		SLATE_EVENT(FSimpleDelegate, OnLeaderboardClicked)
		SLATE_EVENT(FSimpleDelegate, OnSettingsClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Delegates for buttons */
	FSimpleDelegate OnPlayClicked;
	FSimpleDelegate OnShopClicked;
	FSimpleDelegate OnLeaderboardClicked;
	FSimpleDelegate OnSettingsClicked;

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

