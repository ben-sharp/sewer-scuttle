// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ConfigManager.generated.h"

/**
 * Manages configuration settings for the game
 * These can be edited in Project Settings -> Game -> Sewer Scuttle Config
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Sewer Scuttle Config"))
class SEWERSCUTTLE_API UConfigManager : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UConfigManager* Get();

	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetApiBaseUrl() const;

	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetDevAuthToken() const { return DevAuthToken; }

	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetDevDeviceId() const { return DevDeviceId; }

private:
	UPROPERTY(Config, EditAnywhere, Category = "Environment")
	FString ApiBaseUrlDev = TEXT("backend.test/api");

	UPROPERTY(Config, EditAnywhere, Category = "Environment")
	FString ApiBaseUrlProd = TEXT("api.sewerscuttle.com/api");

	UPROPERTY(Config, EditAnywhere, Category = "Environment")
	bool bUseDevEnvironment = true;

	/** 
	 * Developer Auth Token override for testing in Editor.
	 * If set, this token will be used for all API requests when playing in the Editor.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Development", meta = (ConfigRestartRequired = false))
	FString DevAuthToken;

	/** 
	 * Developer Device ID override for testing in Editor.
	 * If set, this ID will be used for all anonymous/guest requests when playing in the Editor.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Development", meta = (ConfigRestartRequired = false))
	FString DevDeviceId;
};

