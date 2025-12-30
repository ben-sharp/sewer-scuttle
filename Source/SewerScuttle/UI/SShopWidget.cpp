// Copyright Epic Games, Inc. All Rights Reserved.

#include "SShopWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"

void SShopWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 30)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Shop")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SAssignNew(CurrencyText, STextBlock)
				.Text(FText::FromString(TEXT("Coins: 0")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Shop items coming soon...")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 20, 0, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Back")))
				.OnClicked(this, &SShopWidget::OnBackButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
		]
	];
}

void SShopWidget::UpdateCurrency(int32 Currency)
{
	if (CurrencyText.IsValid())
	{
		CurrencyText->SetText(FText::FromString(FString::Printf(TEXT("Coins: %d"), Currency)));
	}
}

FReply SShopWidget::OnBackButtonClicked()
{
	OnBackClicked.ExecuteIfBound();
	return FReply::Handled();
}

FReply SShopWidget::OnPurchaseButtonClicked(FString ItemId)
{
	OnPurchaseItem.ExecuteIfBound(ItemId);
	return FReply::Handled();
}

