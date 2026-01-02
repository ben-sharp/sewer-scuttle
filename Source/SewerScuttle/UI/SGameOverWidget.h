// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnRetryClicked);
DECLARE_DELEGATE(FOnChangeClassClicked);
DECLARE_DELEGATE(FOnExitClicked);

/**
 * Slate widget for game over screen
 * Shows final score and provides Retry/Exit options
 */
class SEWERSCUTTLE_API SGameOverWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameOverWidget)
	{}
		SLATE_EVENT(FSimpleDelegate, OnRetryClicked)
		SLATE_EVENT(FSimpleDelegate, OnChangeClassClicked)
		SLATE_EVENT(FSimpleDelegate, OnExitClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 FinalScore, float FinalDistance, float FinalTime);

	/** Delegates */
	FSimpleDelegate OnRetryClicked;
	FSimpleDelegate OnChangeClassClicked;
	FSimpleDelegate OnExitClicked;

private:
	/** Final score text */
	TSharedPtr<class STextBlock> FinalScoreText;

	/** Final distance text */
	TSharedPtr<class STextBlock> FinalDistanceText;

	/** Final time text */
	TSharedPtr<class STextBlock> FinalTimeText;

	/** Retry button callback */
	FReply OnRetryButtonClicked();

	/** Change class button callback */
	FReply OnChangeClassButtonClicked();

	/** Exit button callback */
	FReply OnExitButtonClicked();
};





