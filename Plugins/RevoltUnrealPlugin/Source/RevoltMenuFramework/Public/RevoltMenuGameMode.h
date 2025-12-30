#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/AudioComponent.h"
#include "RevoltMenuGameMode.generated.h"

class URevoltMenuConfig;

class ARevoltMenuManager;

/**
 * GameMode for the Main Menu.
 * Automatically sets the HUD class to ARevoltMenuHUD and configures the input mode.
 */
UCLASS()
class REVOLTMENUFRAMEWORK_API ARevoltMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ARevoltMenuGameMode();

	/** 
	 * Configuration for the menu.
	 * NOTE: This can be set directly in the GameMode (via Blueprint) OR by placing an ARevoltMenuManager actor in the level.
	 * The Manager actor takes precedence.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Revolt Menu")
	URevoltMenuConfig* MenuConfig;

	/** Reference to the manager found in the level */
	UPROPERTY(Transient)
	ARevoltMenuManager* ActiveManager;

protected:
	virtual void BeginPlay() override;

	/** Attempts to find an ARevoltMenuManager in the world and use its config */
	void FindMenuConfigFromManager();

	/** Finds camera and sets view */
	void SetMenuCamera();

	// --- Music System ---
	
	UPROPERTY()
	UAudioComponent* MusicComp;

	UPROPERTY()
	TArray<USoundBase*> Playlist;
	
	int32 CurrentTrackIndex = -1;

	void StartMusic();
	
	UFUNCTION()
	void OnTrackFinished();
};

