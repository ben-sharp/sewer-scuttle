// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EndlessRunner/PlayerClass.h"
#include "EndlessRunnerHUD.generated.h"

struct FTrackSelectionData;
struct FBossRewardData;

/**
 * Main HUD class for the endless runner game
 * Manages Slate widgets for various game states
 */
UCLASS()
class SEWERSCUTTLE_API AEndlessRunnerHUD : public AHUD
{
	GENERATED_BODY()

public:
	AEndlessRunnerHUD();

	virtual void BeginPlay() override;

	/** Show the main menu */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

	/** Show the class selection UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowClassSelection();

	/** Show the in-game HUD (score, distance, lives) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInGameHUD();

	/** Show the track selection UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowTrackSelection();

	/** Show the shop UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowShop();

	/** Show the boss reward UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowBossRewards();

	/** Show the endless mode prompt */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowEndlessModePrompt();

	/** Show the game over screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOverScreen();

	/** Hide all widgets */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllWidgets();

	/** Toggle pause menu */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePause();

	/** Handle shop items received */
	void OnShopItemsReceived(const struct FShopData& ShopData);

protected:
	/** Main menu widget */
	TSharedPtr<class SMainMenuWidget> MainMenuWidget;

	/** Class selection widget */
	TSharedPtr<class SClassSelectionWidget> ClassSelectionWidget;

	/** In-game HUD widget */
	TSharedPtr<class SEndlessRunnerHUD> InGameHUDWidget;

	/** Track selection widget */
	TSharedPtr<class STrackSelectionWidget> TrackSelectionWidget;

	/** Shop widget */
	TSharedPtr<class SShopWidget> ShopWidget;

	/** Boss reward widget */
	TSharedPtr<class SBossRewardWidget> BossRewardWidget;

	/** Endless mode prompt widget */
	TSharedPtr<class SEndlessModePromptWidget> EndlessModePromptWidget;

	/** Game over widget */
	TSharedPtr<class SGameOverWidget> GameOverWidget;

	/** Pause menu widget */
	TSharedPtr<class SPauseWidget> PauseWidget;

	/** Handle track selection */
	void OnTrackSelected(int32 TrackIndex);

	/** Handle menu buttons */
	void OnPlayClicked();
	void OnClassSelected(EPlayerClass SelectedClass);
	void OnShopClicked();
	void OnLeaderboardClicked();
	void OnSettingsClicked();

	/** Handle shop purchase */
	void OnPurchaseItem(FString ItemId);

	/** Handle shop exit */
	void OnShopExited();

	/** Handle reward selection */
	void OnRewardSelected(FString RewardId);

	/** Handle endless mode selection */
	void OnEndlessModeSelected();

	/** Handle endless mode declined */
	void OnEndlessModeDeclined();
};
