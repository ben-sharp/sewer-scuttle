// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnResumeClicked);
DECLARE_DELEGATE(FOnNewRunClicked);
DECLARE_DELEGATE(FOnSettingsClicked);
DECLARE_DELEGATE(FOnMainMenuClicked);

/**
 * Slate widget for pause screen
 * Shows seed and provides Resume/New Run/Settings/Main Menu options
 */
class SEWERSCUTTLE_API SPauseWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPauseWidget)
	{}
		SLATE_EVENT(FSimpleDelegate, OnResumeClicked)
		SLATE_EVENT(FSimpleDelegate, OnNewRunClicked)
		SLATE_EVENT(FSimpleDelegate, OnSettingsClicked)
		SLATE_EVENT(FSimpleDelegate, OnMainMenuClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 Seed);

	/** Delegates */
	FSimpleDelegate OnResumeClicked;
	FSimpleDelegate OnNewRunClicked;
	FSimpleDelegate OnSettingsClicked;
	FSimpleDelegate OnMainMenuClicked;

private:
	/** Seed text */
	TSharedPtr<class STextBlock> SeedText;

	/** Resume button callback */
	FReply OnResumeButtonClicked();

	/** New Run button callback */
	FReply OnNewRunButtonClicked();

	/** Settings button callback */
	FReply OnSettingsButtonClicked();

	/** Main menu button callback */
	FReply OnMainMenuButtonClicked();
};


















