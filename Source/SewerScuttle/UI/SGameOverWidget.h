// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnNewRunClicked);
DECLARE_DELEGATE(FOnChangeClassClicked);
DECLARE_DELEGATE(FOnExitClicked);

/**
 * Slate widget for game over screen
 * Shows final score and provides New Run/Exit options
 */
class SEWERSCUTTLE_API SGameOverWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameOverWidget)
	{}
		SLATE_EVENT(FSimpleDelegate, OnNewRunClicked)
		SLATE_EVENT(FSimpleDelegate, OnChangeClassClicked)
		SLATE_EVENT(FSimpleDelegate, OnExitClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, int32 FinalScore, float FinalDistance, float FinalTime);

	/** Delegates */
	FSimpleDelegate OnNewRunClicked;
	FSimpleDelegate OnChangeClassClicked;
	FSimpleDelegate OnExitClicked;

private:
	/** Final score text */
	TSharedPtr<class STextBlock> FinalScoreText;

	/** Final distance text */
	TSharedPtr<class STextBlock> FinalDistanceText;

	/** Final time text */
	TSharedPtr<class STextBlock> FinalTimeText;

	/** New Run button callback */
	FReply OnNewRunButtonClicked();

	/** Change class button callback */
	FReply OnChangeClassButtonClicked();

	/** Exit button callback */
	FReply OnExitButtonClicked();
};





