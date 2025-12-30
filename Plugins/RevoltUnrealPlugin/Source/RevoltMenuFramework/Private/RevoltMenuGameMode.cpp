#include "RevoltMenuGameMode.h"
#include "RevoltMenuHUD.h"
#include "RevoltMenuConfig.h"
#include "RevoltMenuManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "EngineUtils.h"

ARevoltMenuGameMode::ARevoltMenuGameMode()
{
	// Set the default HUD class to our custom HUD
	HUDClass = ARevoltMenuHUD::StaticClass();
}

void ARevoltMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Try to find config from Manager actor first
	FindMenuConfigFromManager();

	if (!MenuConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltMenuFramework: No MenuConfig assigned! Place an ARevoltMenuManager actor in the level or set it in the GameMode."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: MenuConfig found: %s"), *MenuConfig->GetName());
	
	// 0. Fade Camera to Black immediately to hide loading artifacts
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		// Start fully black, hold black.
		PC->ClientSetCameraFade(true, FColor::Black, FVector2D(1.0f, 1.0f), 0.0f, true);
	}

	// 1. Pass config to HUD
	if (PC)
	{
		if (ARevoltMenuHUD* MyHUD = Cast<ARevoltMenuHUD>(PC->GetHUD()))
		{
			UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: Passing MenuConfig to HUD: %s"), *MyHUD->GetName());
			MyHUD->MenuConfig = MenuConfig;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RevoltMenuFramework: HUD class is not ARevoltMenuHUD! Current HUD: %s"), PC->GetHUD() ? *PC->GetHUD()->GetName() : TEXT("None"));
		}
	}

	// 2. Set Camera in current level
	UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: Searching current level for camera"));
	SetMenuCamera();

	// 3. Start Music
	StartMusic();
}

void ARevoltMenuGameMode::FindMenuConfigFromManager()
{
	// Look for the manager actor
	for (TActorIterator<ARevoltMenuManager> It(GetWorld()); It; ++It)
	{
		ARevoltMenuManager* Manager = *It;
		if (Manager)
		{
			ActiveManager = Manager; // Cache the manager
			if (Manager->MenuConfig)
			{
				MenuConfig = Manager->MenuConfig;
				UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: Using MenuConfig from manager: %s"), *Manager->MenuConfig->GetName());
				return;
			}
		}
	}
}

void ARevoltMenuGameMode::SetMenuCamera()
{
	ACameraActor* TargetCamera = nullptr;

	// 1. Try to get camera from ActiveManager
	if (ActiveManager)
	{
		// Check for cameras
		if (ActiveManager->MenuCameras.Num() > 0)
		{
			// Pick a random one from the list
			int32 RandIndex = FMath::RandRange(0, ActiveManager->MenuCameras.Num() - 1);
			TargetCamera = ActiveManager->MenuCameras[RandIndex];
			
			if (TargetCamera)
			{
				UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: Using Random Camera from Manager (Index %d/%d): %s"), 
					RandIndex, ActiveManager->MenuCameras.Num(), *TargetCamera->GetName());
			}
		}
	}
	// 2. Fallback: Search by tag (deprecated but kept for backward compat if needed, or remove completely)
	/* 
	else if (MenuConfig && !MenuConfig->MenuCameraTag.IsNone())
	{
		// ... removed tag search logic as requested ...
	}
	*/

	if (TargetCamera)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			// INSTANT snap to camera (0.0f blend time) to avoid "flying" effect
			PC->SetViewTargetWithBlend(TargetCamera, 0.0f); 
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RevoltMenuFramework: No MenuCamera assigned in RevoltMenuManager!"));
	}

	// Fade IN from Black to Clear over 1.5 seconds
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->ClientSetCameraFade(true, FColor::Black, FVector2D(1.0f, 0.0f), 1.5f, true);
	}
}

void ARevoltMenuGameMode::StartMusic()
{
	if (!MenuConfig || MenuConfig->MusicPlaylist.Num() == 0) return;

	// Copy playlist
	Playlist = MenuConfig->MusicPlaylist;

	if (MenuConfig->bShuffleMusic)
	{
		// Shuffle
		int32 LastIndex = Playlist.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				Playlist.Swap(i, Index);
			}
		}
	}

	CurrentTrackIndex = -1;
	
	// Create Audio Component
	if (!MusicComp)
	{
		MusicComp = NewObject<UAudioComponent>(this);
		MusicComp->RegisterComponent();
		MusicComp->OnAudioFinished.AddDynamic(this, &ARevoltMenuGameMode::OnTrackFinished);
	}

	OnTrackFinished(); // Start first track
}

void ARevoltMenuGameMode::OnTrackFinished()
{
	if (Playlist.Num() == 0) return;

	CurrentTrackIndex++;
	if (CurrentTrackIndex >= Playlist.Num())
	{
		CurrentTrackIndex = 0; // Loop playlist
	}

	if (Playlist.IsValidIndex(CurrentTrackIndex) && Playlist[CurrentTrackIndex])
	{
		MusicComp->SetSound(Playlist[CurrentTrackIndex]);
		MusicComp->Play();
		UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: Playing Music Track: %s"), *Playlist[CurrentTrackIndex]->GetName());
	}
}
