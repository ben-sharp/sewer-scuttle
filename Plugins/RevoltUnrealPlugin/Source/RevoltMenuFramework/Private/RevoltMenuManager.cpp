#include "RevoltMenuManager.h"
#include "Components/BillboardComponent.h"
#include "Camera/CameraActor.h"
#include "Engine/World.h"

ARevoltMenuManager::ARevoltMenuManager()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Add a sprite so it's easy to find in the editor
	UBillboardComponent* Sprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (Sprite)
	{
		SetRootComponent(Sprite);
	}
#endif
}

void ARevoltMenuManager::SpawnMenuCamera()
{
#if WITH_EDITOR
	if (!GetWorld()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Spawn slightly offset so it's not inside the billboard
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
	FRotator SpawnRotation = GetActorRotation();

	ACameraActor* NewCamera = GetWorld()->SpawnActor<ACameraActor>(SpawnLocation, SpawnRotation, SpawnParams);
	
	if (NewCamera)
	{
		// Auto-add to the list
		MenuCameras.Add(NewCamera);
		
		// Give it a unique label
		int32 Count = MenuCameras.Num();
		NewCamera->SetActorLabel(FString::Printf(TEXT("MenuCamera_%d"), Count));
	}
#endif
}

