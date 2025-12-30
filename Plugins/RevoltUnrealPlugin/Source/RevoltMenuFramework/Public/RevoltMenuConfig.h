#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "RevoltMenuConfig.generated.h"

class UTexture2D;

/**
 * Configuration for the Revolt Main Menu system.
 * Defines the look and feel, as well as the flow of the menu.
 */
UCLASS(BlueprintType)
class REVOLTMENUFRAMEWORK_API URevoltMenuConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Returns the primary asset id for this data asset, used for cooking and asset management */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("RevoltMenuConfig", GetFName());
	}
	/** Logo texture to display at the top of the menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* MenuLogo;

	/** Title text if no logo is provided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText GameTitle = NSLOCTEXT("RevoltMenu", "DefaultTitle", "My Game");
	
	/** Footer text to display at the bottom of the menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FText FooterText = NSLOCTEXT("RevoltMenu", "DefaultFooter", "Powered by Revolt Framework");

	/** The level to load when 'Start Game' is clicked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> StartGameLevel;

	/** List of levels available in the 'Load Level' menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<TSoftObjectPtr<UWorld>> SelectableLevels;

	// ========== VISUALS ==========
	
	// ========== STYLING ==========
	
	/** Main Theme Color (Buttons, Highlights) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor PrimaryColor = FLinearColor(1.0f, 0.2f, 0.4f, 1.0f); // Revolt Red/Pink default

	/** Animation Speed for menu transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float AnimationSpeed = 2.0f;

	// ========== AUDIO ==========

	/** List of background music tracks to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<USoundBase*> MusicPlaylist;

	/** Whether to shuffle the music playlist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bShuffleMusic = false;

	// ========== WEBSITE LINK ==========

	/** Whether to show a website link button at the bottom of the menu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Website Link")
	bool bShowWebsiteButton = false;

	/** Text to display on the website button */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Website Link", meta = (EditCondition = "bShowWebsiteButton"))
	FText WebsiteButtonText = NSLOCTEXT("RevoltMenu", "WebsiteButton", "VISIT WEBSITE");

	/** URL to open when the website button is clicked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Website Link", meta = (EditCondition = "bShowWebsiteButton"))
	FString WebsiteURL = TEXT("https://www.example.com");
};

