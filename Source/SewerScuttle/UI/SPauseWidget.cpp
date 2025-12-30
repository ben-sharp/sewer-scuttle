// Copyright Epic Games, Inc. All Rights Reserved.

#include "SPauseWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateColor.h"
#include "Engine/Engine.h"

void SPauseWidget::Construct(const FArguments& InArgs, int32 Seed)
{
	ChildSlot
	[
		SNew(SOverlay)
		// Dark background overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f)) // Dark semi-transparent
		]
		// Pause content
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
				.Text(FText::FromString(TEXT("PAUSED")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 0.0f))) // Yellow
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SAssignNew(SeedText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("SEED: %d"), Seed)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f))) // Gray
				.Justification(ETextJustify::Center)
			]
			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 15)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("RESUME")))
				.OnClicked(this, &SPauseWidget::OnResumeButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("RESUME")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 15)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("RETRY")))
				.OnClicked(this, &SPauseWidget::OnRetryButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("RETRY")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 15)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("SETTINGS")))
				.OnClicked(this, &SPauseWidget::OnSettingsButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("SETTINGS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("MAIN MENU")))
				.OnClicked(this, &SPauseWidget::OnMainMenuButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("MAIN MENU")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
		]
	];
}

FReply SPauseWidget::OnResumeButtonClicked()
{
	if (OnResumeClicked.IsBound())
	{
		OnResumeClicked.Execute();
	}
	return FReply::Handled();
}

FReply SPauseWidget::OnRetryButtonClicked()
{
	if (OnRetryClicked.IsBound())
	{
		OnRetryClicked.Execute();
	}
	return FReply::Handled();
}

FReply SPauseWidget::OnSettingsButtonClicked()
{
	if (OnSettingsClicked.IsBound())
	{
		OnSettingsClicked.Execute();
	}
	return FReply::Handled();
}

FReply SPauseWidget::OnMainMenuButtonClicked()
{
	if (OnMainMenuClicked.IsBound())
	{
		OnMainMenuClicked.Execute();
	}
	return FReply::Handled();
}


