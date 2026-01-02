// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMainMenuWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"

void SMainMenuWidget::Construct(const FArguments& InArgs)
{
	OnPlayClicked = InArgs._OnPlayClicked;
	OnShopClicked = InArgs._OnShopClicked;
	OnLeaderboardClicked = InArgs._OnLeaderboardClicked;
	OnSettingsClicked = InArgs._OnSettingsClicked;

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
				.Text(FText::FromString(TEXT("Sewer Scuttle")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Play")))
				.OnClicked(this, &SMainMenuWidget::OnPlayButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Shop")))
				.OnClicked(this, &SMainMenuWidget::OnShopButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Leaderboard")))
				.OnClicked(this, &SMainMenuWidget::OnLeaderboardButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Settings")))
				.OnClicked(this, &SMainMenuWidget::OnSettingsButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
		]
	];
}

FReply SMainMenuWidget::OnPlayButtonClicked()
{
	OnPlayClicked.ExecuteIfBound();
	return FReply::Handled();
}

FReply SMainMenuWidget::OnShopButtonClicked()
{
	OnShopClicked.ExecuteIfBound();
	return FReply::Handled();
}

FReply SMainMenuWidget::OnLeaderboardButtonClicked()
{
	OnLeaderboardClicked.ExecuteIfBound();
	return FReply::Handled();
}

FReply SMainMenuWidget::OnSettingsButtonClicked()
{
	OnSettingsClicked.ExecuteIfBound();
	return FReply::Handled();
}

