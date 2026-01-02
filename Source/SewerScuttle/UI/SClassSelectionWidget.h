// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "../EndlessRunner/PlayerClass.h"

class UPlayerClassDefinition;
class AEndlessRunnerGameMode;

DECLARE_DELEGATE_OneParam(FOnClassSelected, EPlayerClass);

/**
 * Class selection widget for choosing player class before starting a run
 */
class SEWERSCUTTLE_API SClassSelectionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SClassSelectionWidget)
	{}
		SLATE_EVENT(FOnClassSelected, OnClassSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Refresh class buttons (call this if data assets are added after construction) */
	void RefreshClassButtons();

	/** Delegate for class selection (public access) */
	FOnClassSelected OnClassSelected;

private:
	/** Handle class selection */
	FReply OnClassButtonClicked(UPlayerClassDefinition* SelectedClassDef);

	/** Populate class buttons from data assets using AssetRegistry */
	void PopulateClassButtons();

	/** Create a stat display widget for a class */
	TSharedRef<SWidget> CreateStatWidget(const FText& Label, const FText& Value, const FSlateColor& Color);

	/** Container for class buttons */
	TSharedPtr<SVerticalBox> ClassListContainer;
};

