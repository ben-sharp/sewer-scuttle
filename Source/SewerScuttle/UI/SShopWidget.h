// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnPurchaseItem, FString);

/**
 * Slate widget for shop interface
 * Displays available items and currency
 */
class SEWERSCUTTLE_API SShopWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShopWidget)
	{}
		SLATE_EVENT(FSimpleDelegate, OnBackClicked)
		SLATE_EVENT(FOnPurchaseItem, OnPurchaseItem)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update currency display */
	void UpdateCurrency(int32 Currency);

	/** Update shop items */
	void UpdateItems(const struct FShopData& ShopData);

	/** Delegates */
	FSimpleDelegate OnBackClicked;
	FOnPurchaseItem OnPurchaseItem;

private:
	/** Currency text */
	TSharedPtr<class STextBlock> CurrencyText;

	/** Items container */
	TSharedPtr<class SVerticalBox> ItemsContainer;

	/** Handle back button click */
	FReply OnBackButtonClicked();

	/** Handle purchase button click */
	FReply OnPurchaseButtonClicked(FString ItemId);
};

