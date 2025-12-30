// Copyright Epic Games, Inc. All Rights Reserved.

#include "SEndlessRunnerHUD.h"
#include "../EndlessRunner/EndlessRunnerGameMode.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Widgets/Layout/SBorder.h"

void SEndlessRunnerHUD::Construct(const FArguments& InArgs, AEndlessRunnerGameMode* InGameMode)
{
	GameMode = InGameMode;

	ChildSlot
	[
		SNew(SOverlay)
		// Top Left - Score and Distance
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(20.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(ScoreText, STextBlock)
				.Text(FText::FromString(TEXT("SCORE: 0")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f))) // Gold
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(DistanceText, STextBlock)
				.Text(FText::FromString(TEXT("DISTANCE: 0m")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(CoinsText, STextBlock)
				.Text(FText::FromString(TEXT("COINS: 0")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f))) // Gold
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(LivesText, STextBlock)
				.Text(FText::FromString(TEXT("LIVES: 3")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f))) // Red
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(JumpCountText, STextBlock)
				.Text(FText::FromString(TEXT("JUMPS: 1")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.8f, 1.0f))) // Light blue
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(PowerUpStatusText, STextBlock)
				.Text(FText::FromString(TEXT("")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.5f))) // Cyan/Green
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 10, 0, 0)
			[
				SAssignNew(SeedText, STextBlock)
				.Text(FText::FromString(TEXT("SEED: 0")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f))) // Gray
			]
		]
		// Top Right - Speed and Timer (cool design)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(20.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 15)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
				.Padding(FMargin(15, 10, 15, 10))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("SPEED")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) // Gray
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(SpeedText, STextBlock)
						.Text(FText::FromString(TEXT("0")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.5f))) // Cyan/Green
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
				.Padding(FMargin(15, 10, 15, 10))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("TIME")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) // Gray
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(TimerText, STextBlock)
						.Text(FText::FromString(TEXT("00:00")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
						.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f))) // White
					]
				]
			]
		]
	];
}

void SEndlessRunnerHUD::UpdateHUD(int32 Score, float Distance, int32 Coins, float Speed, float Time, int32 Lives, int32 CurrentJumpCount, int32 MaxJumpCount, const FString& PowerUpStatus, int32 Seed)
{
	if (ScoreText.IsValid())
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("SCORE: %d"), Score)));
	}

	if (DistanceText.IsValid())
	{
		// Distance is already in meters (converted from game units in GetDistanceTraveled)
		DistanceText->SetText(FText::FromString(FString::Printf(TEXT("DISTANCE: %.0fm"), Distance)));
	}

	if (CoinsText.IsValid())
	{
		CoinsText->SetText(FText::FromString(FString::Printf(TEXT("COINS: %d"), Coins)));
	}

	if (SpeedText.IsValid())
	{
		SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Speed)));
	}

	if (TimerText.IsValid())
	{
		int32 Minutes = FMath::FloorToInt(Time / 60.0f);
		int32 Seconds = FMath::FloorToInt(FMath::Fmod(Time, 60.0f));
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}

	if (LivesText.IsValid())
	{
		FString LivesString = FString::Printf(TEXT("LIVES: %d"), Lives);
		if (Lives <= 0)
		{
			LivesString += TEXT(" (LAST LEGS!)");
			LivesText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f))); // Bright red
		}
		else
		{
			LivesText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f))); // Normal red
		}
		LivesText->SetText(FText::FromString(LivesString));
	}

	if (JumpCountText.IsValid())
	{
		JumpCountText->SetText(FText::FromString(FString::Printf(TEXT("JUMPS: %d/%d"), CurrentJumpCount, MaxJumpCount)));
	}

	if (PowerUpStatusText.IsValid())
	{
		if (!PowerUpStatus.IsEmpty())
		{
			PowerUpStatusText->SetText(FText::FromString(FString::Printf(TEXT("POWERUP: %s"), *PowerUpStatus)));
			PowerUpStatusText->SetVisibility(EVisibility::Visible);
		}
		else
		{
			PowerUpStatusText->SetVisibility(EVisibility::Collapsed);
		}
	}

	if (SeedText.IsValid())
	{
		SeedText->SetText(FText::FromString(FString::Printf(TEXT("SEED: %d"), Seed)));
	}
}

