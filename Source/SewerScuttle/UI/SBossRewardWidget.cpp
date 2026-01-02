// Copyright Epic Games, Inc. All Rights Reserved.

#include "SBossRewardWidget.h"
#include "../EndlessRunner/WebServerInterface.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Styling/CoreStyle.h"

void SBossRewardWidget::Construct(const FArguments& InArgs)
{
	OnRewardSelected = InArgs._OnRewardSelected;

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
				.Text(FText::FromString(TEXT("Choose Your Reward")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20)
			[
				SAssignNew(RewardsContainer, SHorizontalBox)
			]
		]
	];
}

void SBossRewardWidget::UpdateRewards(const TArray<FBossRewardData>& Rewards)
{
	if (!RewardsContainer.IsValid())
	{
		return;
	}

	RewardsContainer->ClearChildren();

	for (const FBossRewardData& Reward : Rewards)
	{
		FString RewardId = Reward.Id;
		
		RewardsContainer->AddSlot()
		.FillWidth(1.0f)
		.Padding(10)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Reward.Name))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20, 10)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Claim")))
				.OnClicked_Lambda([this, RewardId]() { return OnRewardButtonClicked(RewardId); })
				.ContentPadding(FMargin(20, 10))
			]
		];
	}
}

FReply SBossRewardWidget::OnRewardButtonClicked(const FString& RewardId)
{
	if (OnRewardSelected.IsBound())
	{
		OnRewardSelected.Execute(RewardId);
	}
	return FReply::Handled();
}

