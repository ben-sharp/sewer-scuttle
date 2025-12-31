// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnResumeClicked);
DECLARE_DELEGATE(FOnRetryClicked);
DECLARE_DELEGATE(FOnSettingsClicked);
DECLARE_DELEGATE(FOnMainMenuClicked);

/**
 * Slate widget for pause screen
 * Shows seed and provides Resume/Retry/Settings/Main Menu options
 */
class SEWERSCUTTLE_API SPauseWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPauseWidget)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 Seed);

	/** Delegate for resume button */
	FOnResumeClicked OnResumeClicked;

	/** Delegate for retry button */
	FOnRetryClicked OnRetryClicked;

	/** Delegate for settings button */
	FOnSettingsClicked OnSettingsClicked;

	/** Delegate for main menu button */
	FOnMainMenuClicked OnMainMenuClicked;

private:
	/** Seed text */
	TSharedPtr<class STextBlock> SeedText;

	/** Resume button callback */
	FReply OnResumeButtonClicked();

	/** Retry button callback */
	FReply OnRetryButtonClicked();

	/** Settings button callback */
	FReply OnSettingsButtonClicked();

	/** Main menu button callback */
	FReply OnMainMenuButtonClicked();
};
















