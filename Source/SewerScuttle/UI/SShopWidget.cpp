// Copyright Epic Games, Inc. All Rights Reserved.

#include "SShopWidget.h"
#include "../EndlessRunner/WebServerInterface.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Styling/CoreStyle.h"

void SShopWidget::Construct(const FArguments& InArgs)
{
	OnBackClicked = InArgs._OnBackClicked;
	OnPurchaseItem = InArgs._OnPurchaseItem;

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
			.Padding(FMargin(0, 0, 0, 30))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Shop")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SAssignNew(CurrencyText, STextBlock)
				.Text(FText::FromString(TEXT("Coins: 0")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SAssignNew(ItemsContainer, SVerticalBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 20, 0, 0))
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Exit Shop")))
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

void SShopWidget::UpdateItems(const FShopData& ShopData)
{
	if (!ItemsContainer.IsValid()) return;

	ItemsContainer->ClearChildren();

	for (const FShopItemData& Item : ShopData.Items)
	{
		FString ItemId = Item.Id;
		int32 Cost = Item.Cost;

		ItemsContainer->AddSlot()
		.Padding(FMargin(0, 5))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item.Name))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(20, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%d Coins"), Cost)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Buy")))
				.OnClicked_Lambda([this, ItemId]() { return OnPurchaseButtonClicked(ItemId); })
				.ContentPadding(FMargin(15, 5))
			]
		];
	}

	if (ShopData.Items.Num() == 0)
	{
		ItemsContainer->AddSlot()
		.Padding(FMargin(0, 20))
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No items available.")))
			.Justification(ETextJustify::Center)
		];
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

