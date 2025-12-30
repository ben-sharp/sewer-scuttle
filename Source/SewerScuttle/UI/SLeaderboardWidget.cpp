// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLeaderboardWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"

void SLeaderboardWidget::Construct(const FArguments& InArgs)
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
				.Text(FText::FromString(TEXT("Leaderboard")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SAssignNew(LeaderboardContainer, SVerticalBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 20, 0, 20)
			[
				SAssignNew(PlayerRankText, STextBlock)
				.Text(FText::FromString(TEXT("Your Rank: --")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 20, 0, 0)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Back")))
				.OnClicked(this, &SLeaderboardWidget::OnBackButtonClicked)
				.ContentPadding(FMargin(20, 10))
			]
		]
	];
}

void SLeaderboardWidget::UpdateLeaderboard(const TArray<FString>& TopScores, int32 PlayerRank)
{
	if (!LeaderboardContainer.IsValid())
	{
		return;
	}

	// Clear existing entries
	LeaderboardContainer->ClearChildren();

	// Add top scores
	for (int32 i = 0; i < TopScores.Num(); ++i)
	{
		LeaderboardContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%d. %s"), i + 1, *TopScores[i])))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor::White))
		];
	}

	// Update player rank
	if (PlayerRankText.IsValid())
	{
		if (PlayerRank > 0)
		{
			PlayerRankText->SetText(FText::FromString(FString::Printf(TEXT("Your Rank: %d"), PlayerRank)));
		}
		else
		{
			PlayerRankText->SetText(FText::FromString(TEXT("Your Rank: --")));
		}
	}
}

FReply SLeaderboardWidget::OnBackButtonClicked()
{
	OnBackClicked.ExecuteIfBound();
	return FReply::Handled();
}

