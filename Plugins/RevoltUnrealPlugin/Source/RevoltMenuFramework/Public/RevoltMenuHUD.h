#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RevoltMenuHUD.generated.h"

class URevoltMenuConfig;
class SRevoltMainMenu;

/**
 * HUD class to display the Revolt Main Menu.
 * Assign this HUD to your MainMenu GameMode.
 */
UCLASS()
class REVOLTMENUFRAMEWORK_API ARevoltMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Config asset to drive the menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Config")
	URevoltMenuConfig* MenuConfig;

	// Begin AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor interface

	/** Initialize the menu after a delay to allow GameMode setup */
	void InitializeMenu();

private:
	/** Reference to the slate widget */
	TSharedPtr<SRevoltMainMenu> MainMenuWidget;
};

