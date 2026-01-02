// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConfigManager.generated.h"

/**
 * Manages configuration settings for the game
 */
UCLASS()
class SEWERSCUTTLE_API UConfigManager : public UObject
{
	GENERATED_BODY()

public:
	static UConfigManager* Get();

	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetApiBaseUrl() const;

private:
	UPROPERTY(Config)
	FString ApiBaseUrlDev = TEXT("backend.test/api");

	UPROPERTY(Config)
	FString ApiBaseUrlProd = TEXT("api.sewerscuttle.com/api");

	UPROPERTY(Config)
	bool bUseDevEnvironment = true;
};

