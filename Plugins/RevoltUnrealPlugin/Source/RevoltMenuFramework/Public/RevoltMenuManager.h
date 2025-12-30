#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RevoltMenuManager.generated.h"

class URevoltMenuConfig;
class ACameraActor;

/**
 * Manager actor that holds the configuration for the menu.
 * Place this in your level and assign the Data Asset.
 * The RevoltMenuGameMode will look for this actor to configure the menu.
 */
UCLASS()
class REVOLTMENUFRAMEWORK_API ARevoltMenuManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ARevoltMenuManager();

	/** The configuration asset for the menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Revolt Menu")
	URevoltMenuConfig* MenuConfig;

	/** 
	 * List of potential cameras for the menu background.
	 * If one or more are assigned, one will be chosen randomly on startup.
	 * Assign manually or use 'Spawn Menu Camera' button to create and add one.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Revolt Menu")
	TArray<ACameraActor*> MenuCameras;

	/** Spawns a new CameraActor at the manager's location and adds it to the MenuCameras list. */
	UFUNCTION(CallInEditor, Category = "Revolt Menu")
	void SpawnMenuCamera();
};

