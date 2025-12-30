#include "RevoltMenuHUD.h"
#include "RevoltMenuGameMode.h"
#include "SRevoltMainMenu.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"

void ARevoltMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	// Don't immediately create the menu - wait for the next tick to allow GameMode to initialize
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ARevoltMenuHUD::InitializeMenu);
}

void ARevoltMenuHUD::InitializeMenu()
{
	// Check if we have MenuConfig, if not, try to get it from the GameMode
	if (!MenuConfig)
	{
		// Try to get config from the game mode
		if (UWorld* World = GetWorld())
		{
			if (AGameModeBase* GameMode = World->GetAuthGameMode())
			{
				if (ARevoltMenuGameMode* MenuGameMode = Cast<ARevoltMenuGameMode>(GameMode))
				{
					MenuConfig = MenuGameMode->MenuConfig;
				}
			}
		}
	}

	if (!MenuConfig)
	{
		// Try again in 0.5 seconds
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, this, &ARevoltMenuHUD::InitializeMenu, 0.5f, false);
		return;
	}

	MainMenuWidget = SNew(SRevoltMainMenu)
		.MenuConfig(MenuConfig);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget)
		.PossiblyNullContent(MainMenuWidget.ToSharedRef())
	);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void ARevoltMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine && GEngine->GameViewport && MainMenuWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MainMenuWidget.ToSharedRef());
	}

	// Restore input mode to game-only when leaving the menu
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	Super::EndPlay(EndPlayReason);
}

