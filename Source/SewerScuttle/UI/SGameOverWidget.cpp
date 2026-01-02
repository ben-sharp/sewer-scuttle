// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGameOverWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateColor.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void SGameOverWidget::Construct(const FArguments& InArgs, int32 FinalScore, float FinalDistance, float FinalTime)
{
	OnRetryClicked = InArgs._OnRetryClicked;
	OnChangeClassClicked = InArgs._OnChangeClassClicked;
	OnExitClicked = InArgs._OnExitClicked;

	// Format time
	int32 Minutes = FMath::FloorToInt(FinalTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(FMath::Fmod(FinalTime, 60.0f));
	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

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
		// Game Over content
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
				.Text(FText::FromString(TEXT("GAME OVER")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f))) // Red
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 20))
			[
				SAssignNew(FinalScoreText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("FINAL SCORE: %d"), FinalScore)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f))) // Gold
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 10))
			[
				SAssignNew(FinalDistanceText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("DISTANCE: %.0fm"), FinalDistance)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 40))
			[
				SAssignNew(FinalTimeText, STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("TIME: %s"), *TimeString)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 0, 0, 15))
			[
				SNew(SButton)
				.OnClicked(this, &SGameOverWidget::OnRetryButtonClicked)
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
			.Padding(FMargin(0, 0, 0, 15))
			[
				SNew(SButton)
				.OnClicked(this, &SGameOverWidget::OnChangeClassButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("CHANGE CLASS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.OnClicked(this, &SGameOverWidget::OnExitButtonClicked)
				.ContentPadding(FMargin(40, 15))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("EXIT TO MENU")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
				]
			]
		]
	];
}

FReply SGameOverWidget::OnRetryButtonClicked()
{
	if (OnRetryClicked.IsBound())
	{
		OnRetryClicked.Execute();
	}
	return FReply::Handled();
}

FReply SGameOverWidget::OnChangeClassButtonClicked()
{
	if (OnChangeClassClicked.IsBound())
	{
		OnChangeClassClicked.Execute();
	}
	return FReply::Handled();
}

FReply SGameOverWidget::OnExitButtonClicked()
{
	if (OnExitClicked.IsBound())
	{
		OnExitClicked.Execute();
	}
	return FReply::Handled();
}

