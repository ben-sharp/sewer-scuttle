// Copyright Epic Games, Inc. All Rights Reserved.

#include "STrackSelectionWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Styling/SlateColor.h"
#include "Styling/CoreStyle.h"

void STrackSelectionWidget::Construct(const FArguments& InArgs)
{
	OnTrackSelected = InArgs._OnTrackSelected;

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
				SAssignNew(TierTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("Select Track - Tier 1")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(20))
			[
				SAssignNew(TrackCardsContainer, SHorizontalBox)
			]
		]
	];
}

void STrackSelectionWidget::UpdateTracks(const FTrackSelectionData& SelectionData)
{
	CurrentSelectionData = SelectionData;

	if (TierTextBlock.IsValid())
	{
		TierTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Select Track - Tier %d"), SelectionData.Tier)));
	}

	if (!TrackCardsContainer.IsValid())
	{
		return;
	}

	TrackCardsContainer->ClearChildren();

	for (int32 i = 0; i < SelectionData.Tracks.Num(); i++)
	{
		const FTrackInfo& Track = SelectionData.Tracks[i];
		
		TrackCardsContainer->AddSlot()
		.FillWidth(1.0f)
		.Padding(10)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Track %d"), Track.Id + 1)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Length: %dm"), Track.Length)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Shops: %d"), Track.ShopCount)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Boss: %s"), *Track.BossId)))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(20, 10))
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Select")))
				.OnClicked_Lambda([this, i]() { return OnTrackButtonClicked(i); })
				.ContentPadding(FMargin(20, 10))
			]
		];
	}
}

FReply STrackSelectionWidget::OnTrackButtonClicked(int32 TrackIndex)
{
	if (OnTrackSelected.IsBound())
	{
		OnTrackSelected.Execute(TrackIndex);
	}
	return FReply::Handled();
}

