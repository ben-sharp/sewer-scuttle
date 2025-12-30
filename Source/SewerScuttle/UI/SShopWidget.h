// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Slate widget for shop interface
 * Displays available items and currency
 */
class SEWERSCUTTLE_API SShopWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShopWidget)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update currency display */
	void UpdateCurrency(int32 Currency);

	/** Delegate for back button */
	DECLARE_DELEGATE(FOnBackClicked);
	FOnBackClicked OnBackClicked;

	/** Delegate for purchase */
	DECLARE_DELEGATE_OneParam(FOnPurchaseItem, FString);
	FOnPurchaseItem OnPurchaseItem;

private:
	/** Currency text */
	TSharedPtr<class STextBlock> CurrencyText;

	/** Handle back button click */
	FReply OnBackButtonClicked();

	/** Handle purchase button click */
	FReply OnPurchaseButtonClicked(FString ItemId);
};

