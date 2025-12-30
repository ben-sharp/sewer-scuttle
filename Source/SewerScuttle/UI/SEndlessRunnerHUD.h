// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AEndlessRunnerGameMode;

/**
 * Slate widget for in-game HUD
 * Displays score, distance, coins, and powerup indicators
 */
class SEWERSCUTTLE_API SEndlessRunnerHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEndlessRunnerHUD)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, AEndlessRunnerGameMode* InGameMode);

	/** Update HUD display */
	void UpdateHUD(int32 Score, float Distance, int32 Coins, float Speed, float Time, int32 Lives, int32 CurrentJumpCount, int32 MaxJumpCount, const FString& PowerUpStatus, int32 Seed = 0);

private:
	/** Game mode reference */
	TWeakObjectPtr<AEndlessRunnerGameMode> GameMode;

	/** Score text */
	TSharedPtr<class STextBlock> ScoreText;

	/** Distance text */
	TSharedPtr<class STextBlock> DistanceText;

	/** Coins text */
	TSharedPtr<class STextBlock> CoinsText;

	/** Speed text */
	TSharedPtr<class STextBlock> SpeedText;

	/** Timer text */
	TSharedPtr<class STextBlock> TimerText;

	/** Lives text */
	TSharedPtr<class STextBlock> LivesText;

	/** Jump count text */
	TSharedPtr<class STextBlock> JumpCountText;

	/** PowerUp status text */
	TSharedPtr<class STextBlock> PowerUpStatusText;

	/** Seed text (for sharing/reproducibility) */
	TSharedPtr<class STextBlock> SeedText;
};

