// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EndlessRunnerHUD.generated.h"

class SEndlessRunnerHUD;
class SMainMenuWidget;
class SShopWidget;
class SLeaderboardWidget;
class SGameOverWidget;
class SClassSelectionWidget;
class SPauseWidget;
class AEndlessRunnerGameMode;

UENUM(BlueprintType)
enum class EUIState : uint8
{
	MainMenu	UMETA(DisplayName = "Main Menu"),
	InGame		UMETA(DisplayName = "In Game"),
	Shop		UMETA(DisplayName = "Shop"),
	Leaderboard UMETA(DisplayName = "Leaderboard")
};

/**
 * HUD actor that manages Slate widgets
 */
UCLASS()
class SEWERSCUTTLE_API AEndlessRunnerHUD : public AHUD
{
	GENERATED_BODY()

public:
	AEndlessRunnerHUD();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Show main menu */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenu();

	/** Show in-game HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowInGameHUD();

	/** Show shop */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowShop();

	/** Show leaderboard */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLeaderboard();

	/** Show game over screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOverScreen();

	/** Show class selection screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowClassSelection();

	/** Show pause screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseScreen();

	/** Hide pause screen and resume game */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseScreen();

	/** Toggle pause (call from input) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePause();

	/** Get current UI state */
	UFUNCTION(BlueprintPure, Category = "UI")
	EUIState GetUIState() const { return CurrentUIState; }

protected:
	/** Current UI state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	EUIState CurrentUIState = EUIState::MainMenu;

	/** Flag to show main menu on first tick (when viewport is ready) */
	bool bNeedsToShowMainMenu = false;

	/** In-game HUD widget */
	TSharedPtr<SEndlessRunnerHUD> InGameHUDWidget;

	/** Main menu widget */
	TSharedPtr<SMainMenuWidget> MainMenuWidget;

	/** Shop widget */
	TSharedPtr<SShopWidget> ShopWidget;

	/** Leaderboard widget */
	TSharedPtr<SLeaderboardWidget> LeaderboardWidget;

	/** Game over widget */
	TSharedPtr<SGameOverWidget> GameOverWidget;

	/** Class selection widget */
	TSharedPtr<SClassSelectionWidget> ClassSelectionWidget;

	/** Pause widget */
	TSharedPtr<SPauseWidget> PauseWidget;

	/** Create and show widget */
	void CreateAndShowWidget(TSharedPtr<class SWidget> Widget);

	/** Remove all widgets */
	void RemoveAllWidgets();

private:
	/** HUD update timer (throttle updates for performance) */
	float HUDUpdateTimer = 0.0f;
	
	/** HUD update interval (update every 0.1 seconds instead of every frame) */
	static constexpr float HUDUpdateInterval = 0.1f;
};

