// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnEndlessModeSelected);
DECLARE_DELEGATE(FOnEndlessModeDeclined);

/**
 * Prompt for entering endless mode after defeating the final boss
 */
class SEWERSCUTTLE_API SEndlessModePromptWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEndlessModePromptWidget)
	{}
		SLATE_EVENT(FOnEndlessModeSelected, OnEndlessModeSelected)
		SLATE_EVENT(FOnEndlessModeDeclined, OnEndlessModeDeclined)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FOnEndlessModeSelected OnEndlessModeSelected;
	FOnEndlessModeDeclined OnEndlessModeDeclined;

private:
	FReply OnEndlessClicked();
	FReply OnQuitClicked();
};

