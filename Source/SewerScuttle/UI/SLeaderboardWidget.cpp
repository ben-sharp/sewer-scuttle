// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLeaderboardWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Styling/SlateColor.h"
#include "EndlessRunner/PlayerClass.h"

void SLeaderboardWidget::Construct(const FArguments& InArgs)
{
	OnBackClicked = InArgs._OnBackClicked;
	OnWatchReplayClicked = InArgs._OnWatchReplayClicked;
	OnClassTabChanged = InArgs._OnClassTabChanged;
	SelectedClass = TEXT(""); // Default to Overall

	RebuildUI();
}

void SLeaderboardWidget::RebuildUI()
{
	ChildSlot
	[
		SNew(SOverlay)
		// Background Dim
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.85f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(1000)
			.HeightOverride(800)
			[
				SAssignNew(MainStack, SVerticalBox)
				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 20, 0, 30)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("SEWER SCUTTLE HALL OF FAME")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 42))
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f))) // Gold
					.Justification(ETextJustify::Center)
				]
				
				// Tabs Row
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 20)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth() [ CreateClassTab(TEXT(""), FText::FromString(TEXT("OVERALL"))) ]
						+ SHorizontalBox::Slot().AutoWidth() [ CreateClassTab(TEXT("Vanilla"), FText::FromString(TEXT("VANILLA"))) ]
						+ SHorizontalBox::Slot().AutoWidth() [ CreateClassTab(TEXT("Rogue"), FText::FromString(TEXT("ROGUE"))) ]
						+ SHorizontalBox::Slot().AutoWidth() [ CreateClassTab(TEXT("Enforcer"), FText::FromString(TEXT("ENFORCER"))) ]
						+ SHorizontalBox::Slot().AutoWidth() [ CreateClassTab(TEXT("Joker"), FText::FromString(TEXT("JOKER"))) ]
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
				]

				// Header Row
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(40, 0, 40, 10)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(1, 1, 1, 0.1f))
					.Padding(FMargin(20, 10))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 20, 0) [ SNew(STextBlock).Text(FText::FromString(TEXT("RANK"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
						+ SHorizontalBox::Slot().FillWidth(1.0f) [ SNew(STextBlock).Text(FText::FromString(TEXT("PLAYER"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
						+ SHorizontalBox::Slot().AutoWidth().Padding(20, 0, 20, 0) [ SNew(STextBlock).Text(FText::FromString(TEXT("SCORE"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
						+ SHorizontalBox::Slot().AutoWidth() [ SNew(STextBlock).Text(FText::FromString(TEXT("REPLAY"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14)) ]
					]
				]

				// Leaderboard Content
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(40, 0, 40, 0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(LeaderboardContainer, SVerticalBox)
						]
					]
					
					// Loading Overlay
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(LoadingWidget, SBox)
						.Visibility(EVisibility::Collapsed)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("FETCHING DATA...")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
							.ColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1, 0.5f)))
						]
					]
				]

				// Footer / Player Rank
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 20, 0, 20)
				[
					SAssignNew(PlayerRankText, STextBlock)
					.Text(FText::FromString(TEXT("YOUR RANK: --")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
					.Justification(ETextJustify::Center)
				]

				// Back Button
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 40)
				[
					SNew(SButton)
					.OnClicked(this, &SLeaderboardWidget::OnBackButtonClicked)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ContentPadding(FMargin(40, 15))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("RETURN TO MENU")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SLeaderboardWidget::CreateClassTab(const FString& ClassName, const FText& DisplayName)
{
	bool bIsSelected = (SelectedClass == ClassName);

	return SNew(SButton)
		.OnClicked(this, &SLeaderboardWidget::OnTabClicked, ClassName)
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.ContentPadding(FMargin(15, 8))
		.IsEnabled_Lambda([this]() { return LoadingWidget.IsValid() && LoadingWidget->GetVisibility() == EVisibility::Collapsed; })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(DisplayName)
				.Font(FCoreStyle::GetDefaultFontStyle(bIsSelected ? "Bold" : "Regular", 14))
				.ColorAndOpacity(bIsSelected ? FLinearColor(1, 0.8f, 0) : FLinearColor::White)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2, 0, 0)
			[
				SNew(SBox)
				.HeightOverride(2)
				.Visibility(bIsSelected ? EVisibility::Visible : EVisibility::Hidden)
				[
					SNew(SImage)
					.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.ColorAndOpacity(FLinearColor(1, 0.8f, 0))
				]
			]
		];
}

FReply SLeaderboardWidget::OnTabClicked(FString ClassName)
{
	if (SelectedClass != ClassName)
	{
		SelectedClass = ClassName;
		SetLoading(true);
		
		// Re-construct UI to update selection visual before firing the delegate
		RebuildUI();
		
		if (OnClassTabChanged.IsBound())
		{
			OnClassTabChanged.Execute(ClassName);
		}
	}
	return FReply::Handled();
}

void SLeaderboardWidget::SetLoading(bool bLoading)
{
	if (LoadingWidget.IsValid())
	{
		LoadingWidget->SetVisibility(bLoading ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (LeaderboardContainer.IsValid())
	{
		LeaderboardContainer->SetVisibility(bLoading ? EVisibility::HitTestInvisible : EVisibility::Visible);
		if (bLoading) LeaderboardContainer->ClearChildren();
	}
}

void SLeaderboardWidget::UpdateLeaderboard(const TArray<FLeaderboardEntryData>& Entries, int32 PlayerRank)
{
	SetLoading(false);

	if (!LeaderboardContainer.IsValid()) return;

	LeaderboardContainer->ClearChildren();

	if (Entries.Num() == 0)
	{
		LeaderboardContainer->AddSlot()
		.HAlign(HAlign_Center)
		.Padding(0, 50, 0, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("NO RECORDS FOUND IN THIS CATEGORY")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1, 0.3f)))
		];
		return;
	}

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		const FLeaderboardEntryData& Entry = Entries[i];
		FLinearColor RowColor = (i % 2 == 0) ? FLinearColor(1, 1, 1, 0.05f) : FLinearColor(0, 0, 0, 0.2f);
		if (i == 0) RowColor = FLinearColor(1, 0.8f, 0, 0.15f); // Gold highlight for #1

		LeaderboardContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 2)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(RowColor)
			.Padding(FMargin(20, 12))
			[
				SNew(SHorizontalBox)
				// Rank
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 20, 0)
				[
					SNew(SBox)
					.WidthOverride(40)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%d"), i + 1)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(i < 3 ? FLinearColor(1, 0.8f, 0) : FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
				// Player Name & Class
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Entry.PlayerName))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(FPlayerClassData::PlayerClassToString(Entry.PlayerClass)))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
					]
				]
				// Score
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(20, 0, 40, 0)
				[
					SNew(STextBlock)
					.Text(FText::AsNumber(Entry.Score))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
					.ColorAndOpacity(FLinearColor(1, 0.8f, 0))
				]
				// Replay Button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("WATCH")))
					.Visibility(Entry.bHasReplay ? EVisibility::Visible : EVisibility::Collapsed)
					.OnClicked_Lambda([this, Entry]() {
						OnWatchReplayClicked.ExecuteIfBound(Entry.RunId);
						return FReply::Handled();
					})
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.ContentPadding(FMargin(15, 5))
				]
			]
		];
	}

	if (PlayerRankText.IsValid())
	{
		if (PlayerRank > 0)
		{
			PlayerRankText->SetText(FText::FromString(FString::Printf(TEXT("YOUR CURRENT RANK: %d"), PlayerRank)));
			PlayerRankText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			PlayerRankText->SetVisibility(EVisibility::Hidden);
		}
	}
}

FReply SLeaderboardWidget::OnBackButtonClicked()
{
	OnBackClicked.ExecuteIfBound();
	return FReply::Handled();
}

