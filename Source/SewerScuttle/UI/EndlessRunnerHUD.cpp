// Copyright Epic Games, Inc. All Rights Reserved.

#include "EndlessRunnerHUD.h"
#include "SMainMenuWidget.h"
#include "SClassSelectionWidget.h"
#include "SEndlessRunnerHUD.h"
#include "STrackSelectionWidget.h"
#include "SShopWidget.h"
#include "SBossRewardWidget.h"
#include "SEndlessModePromptWidget.h"
#include "SGameOverWidget.h"
#include "SPauseWidget.h"
#include "EndlessRunner/EndlessRunnerGameMode.h"
#include "EndlessRunner/WebServerInterface.h"
#include "EndlessRunner/CurrencyManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

AEndlessRunnerHUD::AEndlessRunnerHUD()
{
}

void AEndlessRunnerHUD::BeginPlay()
{
	Super::BeginPlay();
	ShowMainMenu();
}

void AEndlessRunnerHUD::ShowMainMenu()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(MainMenuWidget, SMainMenuWidget)
			.OnPlayClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnPlayClicked))
			.OnShopClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnShopClicked))
			.OnLeaderboardClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnLeaderboardClicked))
			.OnSettingsClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnSettingsClicked));

		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(MainMenuWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowInGameHUD()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(InGameHUDWidget, SEndlessRunnerHUD, Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()));
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(InGameHUDWidget.ToSharedRef()));
	}
}

void AEndlessRunnerHUD::ShowClassSelection()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(ClassSelectionWidget, SClassSelectionWidget)
			.OnClassSelected(FOnClassSelected::CreateUObject(this, &AEndlessRunnerHUD::OnClassSelected));
		
		ClassSelectionWidget->RefreshClassButtons();
		
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ClassSelectionWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowTrackSelection()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(TrackSelectionWidget, STrackSelectionWidget)
			.OnTrackSelected(FOnTrackSelected::CreateUObject(this, &AEndlessRunnerHUD::OnTrackSelected));
		
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
		{
			TrackSelectionWidget->UpdateTracks(GM->GetCurrentTrackSelection());
		}
		
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(TrackSelectionWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowShop()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(ShopWidget, SShopWidget)
			.OnBackClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnShopExited));

		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (GM->GetCurrencyManager())
			{
				ShopWidget->UpdateCurrency(GM->GetCurrencyManager()->GetCurrency());
			}
		}

		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(ShopWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowBossRewards()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(BossRewardWidget, SBossRewardWidget)
			.OnRewardSelected(FOnRewardSelected::CreateUObject(this, &AEndlessRunnerHUD::OnRewardSelected));
		
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
		{
			BossRewardWidget->UpdateRewards(GM->GetBossRewards());
		}
		
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(BossRewardWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowEndlessModePrompt()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		SAssignNew(EndlessModePromptWidget, SEndlessModePromptWidget)
			.OnEndlessModeSelected(FOnEndlessModeSelected::CreateUObject(this, &AEndlessRunnerHUD::OnEndlessModeSelected))
			.OnEndlessModeDeclined(FOnEndlessModeDeclined::CreateUObject(this, &AEndlessRunnerHUD::OnEndlessModeDeclined));
		
		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(EndlessModePromptWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::ShowGameOverScreen()
{
	HideAllWidgets();
	if (GEngine && GEngine->GameViewport)
	{
		int32 Score = 0;
		float Distance = 0.0f;
		float Time = 0.0f;
		
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
		{
			Score = GM->GetScore();
			Distance = GM->GetDistanceTraveled();
			Time = GM->GetGameTime();
		}
		
		SAssignNew(GameOverWidget, SGameOverWidget, Score, Distance, Time)
			.OnRetryClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnPlayClicked))
			.OnExitClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::ShowMainMenu));

		GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(GameOverWidget.ToSharedRef()));
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AEndlessRunnerHUD::HideAllWidgets()
{
	if (GEngine && GEngine->GameViewport)
	{
		if (MainMenuWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(MainMenuWidget.ToSharedRef());
			MainMenuWidget.Reset();
		}
		if (ClassSelectionWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ClassSelectionWidget.ToSharedRef());
			ClassSelectionWidget.Reset();
		}
		if (InGameHUDWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(InGameHUDWidget.ToSharedRef());
			InGameHUDWidget.Reset();
		}
		if (TrackSelectionWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(TrackSelectionWidget.ToSharedRef());
			TrackSelectionWidget.Reset();
		}
		if (ShopWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ShopWidget.ToSharedRef());
			ShopWidget.Reset();
		}
		if (BossRewardWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(BossRewardWidget.ToSharedRef());
			BossRewardWidget.Reset();
		}
		if (EndlessModePromptWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(EndlessModePromptWidget.ToSharedRef());
			EndlessModePromptWidget.Reset();
		}
		if (GameOverWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(GameOverWidget.ToSharedRef());
			GameOverWidget.Reset();
		}
		if (PauseWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PauseWidget.ToSharedRef());
			PauseWidget.Reset();
		}
	}
}

void AEndlessRunnerHUD::TogglePause()
{
	if (PauseWidget.IsValid())
	{
		HideAllWidgets();
		ShowInGameHUD();
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->ResumeGame();
		}
	}
	else
	{
		if (GEngine && GEngine->GameViewport)
		{
			int32 Seed = 0;
			if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
			{
				Seed = GM->GetTrackSeed();
				GM->PauseGame();
			}

			SAssignNew(PauseWidget, SPauseWidget, Seed)
				.OnResumeClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::TogglePause))
				.OnRetryClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::OnPlayClicked))
				.OnMainMenuClicked(FSimpleDelegate::CreateUObject(this, &AEndlessRunnerHUD::ShowMainMenu));

			GEngine->GameViewport->AddViewportWidgetContent(SNew(SWeakWidget).PossiblyNullContent(PauseWidget.ToSharedRef()));
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->bShowMouseCursor = true;
				PC->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}

void AEndlessRunnerHUD::OnTrackSelected(int32 TrackIndex)
{
	if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->SelectTrack(TrackIndex);
	}
}

void AEndlessRunnerHUD::OnPlayClicked()
{
	ShowClassSelection();
}

void AEndlessRunnerHUD::OnClassSelected(EPlayerClass SelectedClass)
{
	HideAllWidgets();
	if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->SetSelectedClass(SelectedClass);
		GM->StartGame();
	}
}

void AEndlessRunnerHUD::OnShopClicked()
{
	ShowShop();
}

void AEndlessRunnerHUD::OnLeaderboardClicked()
{
	// TODO: Show Leaderboard
}

void AEndlessRunnerHUD::OnSettingsClicked()
{
	// TODO: Show Settings
}

void AEndlessRunnerHUD::OnShopItemsReceived(const FShopData& ShopData)
{
	if (ShopWidget.IsValid())
	{
		ShopWidget->UpdateItems(ShopData);
	}
}

void AEndlessRunnerHUD::OnShopExited()
{
	if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ExitShop();
	}
}

void AEndlessRunnerHUD::OnRewardSelected(FString RewardId)
{
	if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->SelectBossReward(RewardId);
	}
}

void AEndlessRunnerHUD::OnEndlessModeSelected()
{
	if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->StartEndlessMode();
	}
}

void AEndlessRunnerHUD::OnEndlessModeDeclined()
{
	ShowMainMenu();
}
