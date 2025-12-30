// Copyright Epic Games, Inc. All Rights Reserved.

#include "EndlessRunnerHUD.h"
#include "SEndlessRunnerHUD.h"
#include "SMainMenuWidget.h"
#include "SShopWidget.h"
#include "SLeaderboardWidget.h"
#include "SGameOverWidget.h"
#include "SClassSelectionWidget.h"
#include "SPauseWidget.h"
#include "../EndlessRunner/EndlessRunnerGameMode.h"
#include "../EndlessRunner/CurrencyManager.h"
#include "../EndlessRunner/RabbitCharacter.h"
#include "../EndlessRunner/PlayerClass.h"
#include "Engine/World.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AEndlessRunnerHUD::AEndlessRunnerHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentUIState = EUIState::MainMenu;
}

void AEndlessRunnerHUD::BeginPlay()
{
	Super::BeginPlay();

	// Don't show menu immediately - wait for viewport to be ready
	// We'll show it in Tick() on the first frame
	bNeedsToShowMainMenu = true;
}

void AEndlessRunnerHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Show main menu on first tick when viewport is ready
	if (bNeedsToShowMainMenu)
	{
		if (GEngine && GEngine->GameViewport)
		{
			ShowMainMenu();
			bNeedsToShowMainMenu = false;
			UE_LOG(LogTemp, Warning, TEXT("HUD: Showing main menu (viewport ready)"));
		}
	}

	// Throttle HUD updates for performance (update every 0.1 seconds instead of every frame)
	if (CurrentUIState == EUIState::InGame && InGameHUDWidget.IsValid())
	{
		HUDUpdateTimer += DeltaTime;
		if (HUDUpdateTimer >= HUDUpdateInterval)
		{
			HUDUpdateTimer = 0.0f;
			
			AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode)
			{
				int32 Score = GameMode->GetScore();
				float Distance = GameMode->GetDistanceTraveled();
				float GameTime = GameMode->GetGameTime();
				int32 Lives = GameMode->GetLives();
				int32 Coins = GameMode->GetRunCurrency(); // Use run currency (temporary, per-run)

				// Get player speed, jump count, and powerup status
				float Speed = GameMode->GetPlayerSpeed();
				int32 CurrentJumpCount = GameMode->GetCurrentJumpCount();
				int32 MaxJumpCount = GameMode->GetMaxJumpCount();
				FString PowerUpStatus = GameMode->GetActivePowerUpStatus();
				int32 Seed = GameMode->GetTrackSeed();

				InGameHUDWidget->UpdateHUD(Score, Distance, Coins, Speed, GameTime, Lives, CurrentJumpCount, MaxJumpCount, PowerUpStatus, Seed);
			}
		}
	}
}

void AEndlessRunnerHUD::ShowMainMenu()
{
	RemoveAllWidgets();

	// Pause the game when showing main menu
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			// Destroy any existing pawn to prevent it from being active during menu
			if (APawn* ExistingPawn = PlayerController->GetPawn())
			{
				PlayerController->UnPossess();
				ExistingPawn->Destroy();
				UE_LOG(LogTemp, Log, TEXT("HUD: Destroyed existing pawn when showing main menu"));
			}
			
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}

	if (!MainMenuWidget.IsValid())
	{
		MainMenuWidget = SNew(SMainMenuWidget);
		
		// Set up delegates
		MainMenuWidget->OnPlayClicked.BindLambda([this]()
		{
			// Show class selection instead of directly starting game
			ShowClassSelection();
		});

		MainMenuWidget->OnShopClicked.BindLambda([this]()
		{
			ShowShop();
		});

		MainMenuWidget->OnLeaderboardClicked.BindLambda([this]()
		{
			ShowLeaderboard();
		});
	}

	CreateAndShowWidget(MainMenuWidget);
	CurrentUIState = EUIState::MainMenu;
}

void AEndlessRunnerHUD::ShowInGameHUD()
{
	UE_LOG(LogTemp, Warning, TEXT("HUD: ShowInGameHUD() called - Current UI state: %d"), (int32)CurrentUIState);
	
	RemoveAllWidgets();
	
	// Ensure game over widget is cleared and reset
	GameOverWidget.Reset();
	
	// Double-check game state - don't show game over screen if game is starting
	AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		// Force game state to Playing if it's GameOver (shouldn't happen, but safety check)
		EGameState CurrentState = GameMode->GetGameState();
		if (CurrentState == EGameState::GameOver)
		{
			UE_LOG(LogTemp, Error, TEXT("HUD: ShowInGameHUD() - Game state is GameOver! This should not happen. Forcing to Playing."));
			GameMode->SetGameState(EGameState::Playing);
		}
	}

	if (!InGameHUDWidget.IsValid())
	{
		InGameHUDWidget = SNew(SEndlessRunnerHUD, GameMode);
	}

	CreateAndShowWidget(InGameHUDWidget);
	CurrentUIState = EUIState::InGame;
	
	UE_LOG(LogTemp, Warning, TEXT("HUD: ShowInGameHUD() - In-game HUD shown, UI state set to InGame"));
}

void AEndlessRunnerHUD::ShowShop()
{
	RemoveAllWidgets();

	// Pause the game when showing shop
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}

	if (!ShopWidget.IsValid())
	{
		ShopWidget = SNew(SShopWidget);
		
		ShopWidget->OnBackClicked.BindLambda([this]()
		{
			ShowMainMenu();
		});

		// Update currency
		AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode && GameMode->GetCurrencyManager())
		{
			int32 Currency = 0; // TODO: Get from currency manager
			ShopWidget->UpdateCurrency(Currency);
		}
	}

	CreateAndShowWidget(ShopWidget);
	CurrentUIState = EUIState::Shop;
}

void AEndlessRunnerHUD::ShowClassSelection()
{
	RemoveAllWidgets();

	// Pause the game when showing class selection
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}

	if (!ClassSelectionWidget.IsValid())
	{
		ClassSelectionWidget = SNew(SClassSelectionWidget);
		
		// Set up delegate for class selection
		ClassSelectionWidget->OnClassSelected.BindLambda([this](EPlayerClass SelectedClass)
		{
			// Store selected class in GameMode
			AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
			if (GameMode)
			{
				GameMode->SetSelectedClass(SelectedClass);
				UE_LOG(LogTemp, Warning, TEXT("HUD: Class selected: %d"), (int32)SelectedClass);
				
				// Remove class selection widget
				RemoveAllWidgets();
				
				// Start the game (this will apply class perks and show in-game HUD)
				GameMode->StartGame();
			}
		});
	}

	// Always refresh class buttons when showing (in case data assets were added/updated)
	ClassSelectionWidget->RefreshClassButtons();
	
	// Refresh class buttons (uses AssetRegistry, no GameMode needed)
	ClassSelectionWidget->RefreshClassButtons();
	
	CreateAndShowWidget(ClassSelectionWidget);
	
	CurrentUIState = EUIState::MainMenu; // Keep as MainMenu state for now
}

void AEndlessRunnerHUD::ShowLeaderboard()
{
	RemoveAllWidgets();

	// Pause the game when showing leaderboard
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}

	if (!LeaderboardWidget.IsValid())
	{
		LeaderboardWidget = SNew(SLeaderboardWidget);
		
		LeaderboardWidget->OnBackClicked.BindLambda([this]()
		{
			ShowMainMenu();
		});

		// TODO: Load leaderboard data from web server
		TArray<FString> TopScores;
		TopScores.Add(TEXT("Player1 - 10000"));
		TopScores.Add(TEXT("Player2 - 8000"));
		TopScores.Add(TEXT("Player3 - 6000"));
		LeaderboardWidget->UpdateLeaderboard(TopScores, 0);
	}

	CreateAndShowWidget(LeaderboardWidget);
	CurrentUIState = EUIState::Leaderboard;
}

void AEndlessRunnerHUD::ShowGameOverScreen()
{
	RemoveAllWidgets();

	AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	int32 FinalScore = GameMode->GetScore();
	float FinalDistance = GameMode->GetDistanceTraveled();
	float FinalTime = GameMode->GetGameTime();

	if (!GameOverWidget.IsValid())
	{
		GameOverWidget = SNew(SGameOverWidget, FinalScore, FinalDistance, FinalTime);
		
		// Set up delegates
		GameOverWidget->OnRetryClicked.BindLambda([this, GameMode]()
		{
			// Retry - clear everything and restart the game
			RemoveAllWidgets();
			GameOverWidget.Reset();
			GameMode->StartGame();
			ShowInGameHUD();
		});

		GameOverWidget->OnChangeClassClicked.BindLambda([this, GameMode]()
		{
			// Change class - show class selection screen
			RemoveAllWidgets();
			GameOverWidget.Reset();
			ShowClassSelection();
		});

		GameOverWidget->OnExitClicked.BindLambda([this]()
		{
			// Exit to main menu
			ShowMainMenu();
		});
	}
	else
	{
		// Update widget with new scores
		GameOverWidget = SNew(SGameOverWidget, FinalScore, FinalDistance, FinalTime);
		GameOverWidget->OnRetryClicked.BindLambda([this, GameMode]()
		{
			// Retry - clear everything and restart the game
			RemoveAllWidgets();
			GameOverWidget.Reset();
			GameMode->StartGame();
			ShowInGameHUD();
		});

		GameOverWidget->OnChangeClassClicked.BindLambda([this, GameMode]()
		{
			// Change class - show class selection screen
			RemoveAllWidgets();
			GameOverWidget.Reset();
			ShowClassSelection();
		});

		GameOverWidget->OnExitClicked.BindLambda([this]()
		{
			ShowMainMenu();
		});
	}

	CreateAndShowWidget(GameOverWidget);
}

void AEndlessRunnerHUD::ShowPauseScreen()
{
	AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: ShowPauseScreen - GameMode is null"));
		return;
	}

	// Only show pause if game is actually playing
	if (GameMode->GetGameState() != EGameState::Playing)
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD: ShowPauseScreen - Game is not in Playing state, ignoring"));
		return;
	}

	// Pause the game
	GameMode->PauseGame();

	// Get current seed
	int32 Seed = GameMode->GetTrackSeed();

	if (!PauseWidget.IsValid())
	{
		PauseWidget = SNew(SPauseWidget, Seed);
		
		// Set up delegates
		PauseWidget->OnResumeClicked.BindLambda([this, GameMode]()
		{
			HidePauseScreen();
		});

		PauseWidget->OnRetryClicked.BindLambda([this, GameMode]()
		{
			// Retry - clear everything and restart the game
			RemoveAllWidgets();
			PauseWidget.Reset();
			GameMode->StartGame();
			ShowInGameHUD();
		});

		PauseWidget->OnSettingsClicked.BindLambda([this]()
		{
			// TODO: Show settings screen
			UE_LOG(LogTemp, Warning, TEXT("HUD: Settings button clicked - not implemented yet"));
		});

		PauseWidget->OnMainMenuClicked.BindLambda([this]()
		{
			// Exit to main menu
			RemoveAllWidgets();
			PauseWidget.Reset();
			ShowMainMenu();
		});
	}
	else
	{
		// Update seed in case it changed
		PauseWidget = SNew(SPauseWidget, Seed);
		PauseWidget->OnResumeClicked.BindLambda([this, GameMode]()
		{
			HidePauseScreen();
		});

		PauseWidget->OnRetryClicked.BindLambda([this, GameMode]()
		{
			RemoveAllWidgets();
			PauseWidget.Reset();
			GameMode->StartGame();
			ShowInGameHUD();
		});

		PauseWidget->OnSettingsClicked.BindLambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("HUD: Settings button clicked - not implemented yet"));
		});

		PauseWidget->OnMainMenuClicked.BindLambda([this]()
		{
			RemoveAllWidgets();
			PauseWidget.Reset();
			ShowMainMenu();
		});
	}

	// Show mouse cursor and enable UI input
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}

	CreateAndShowWidget(PauseWidget);
}

void AEndlessRunnerHUD::HidePauseScreen()
{
	if (PauseWidget.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PauseWidget.ToSharedRef());
		}
		PauseWidget.Reset();
	}

	// Resume the game
	AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ResumeGame();
	}

	// Hide mouse cursor and restore game input
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
}

void AEndlessRunnerHUD::TogglePause()
{
	AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	// Only allow pause/unpause if game is playing or paused
	EGameState CurrentState = GameMode->GetGameState();
	if (CurrentState == EGameState::Playing)
	{
		ShowPauseScreen();
	}
	else if (CurrentState == EGameState::Paused)
	{
		HidePauseScreen();
	}
}

void AEndlessRunnerHUD::CreateAndShowWidget(TSharedPtr<class SWidget> Widget)
{
	if (!Widget.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: CreateAndShowWidget called with invalid widget!"));
		return;
	}
	
	if (!GEngine)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: GEngine is null!"));
		return;
	}
	
	if (!GEngine->GameViewport)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: GameViewport is null! Viewport not ready yet."));
		return;
	}
	
	GEngine->GameViewport->AddViewportWidgetContent(Widget.ToSharedRef(), 1000);
	UE_LOG(LogTemp, Log, TEXT("HUD: Added widget to viewport (ZOrder=1000)"));
}

void AEndlessRunnerHUD::RemoveAllWidgets()
{
	if (GEngine && GEngine->GameViewport)
	{
		if (InGameHUDWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(InGameHUDWidget.ToSharedRef());
		}
		if (MainMenuWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(MainMenuWidget.ToSharedRef());
		}
		if (ShopWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ShopWidget.ToSharedRef());
		}
		if (LeaderboardWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(LeaderboardWidget.ToSharedRef());
		}
		if (GameOverWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(GameOverWidget.ToSharedRef());
		}
		if (ClassSelectionWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ClassSelectionWidget.ToSharedRef());
		}
		if (PauseWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PauseWidget.ToSharedRef());
		}
	}
}

