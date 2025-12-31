// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConfigManager.generated.h"

/**
 * Configuration manager for API settings
 * Reads from DefaultGame.ini and provides runtime configuration
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UConfigManager : public UObject
{
	GENERATED_BODY()

public:
	/** Get singleton instance */
	UFUNCTION(BlueprintCallable, Category = "Config")
	static UConfigManager* Get();

	/** Initialize configuration (call once at startup) */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void Initialize();

	/** Get base API URL (dev or prod based on environment) */
	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetApiBaseUrl() const { return bUseDevEnvironment ? ApiBaseUrlDev : ApiBaseUrlProd; }

	/** Get dev API URL */
	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetApiBaseUrlDev() const { return ApiBaseUrlDev; }

	/** Get prod API URL */
	UFUNCTION(BlueprintPure, Category = "Config")
	FString GetApiBaseUrlProd() const { return ApiBaseUrlProd; }

	/** Check if using dev environment */
	UFUNCTION(BlueprintPure, Category = "Config")
	bool IsDevEnvironment() const { return bUseDevEnvironment; }

	/** Get token expiration days */
	UFUNCTION(BlueprintPure, Category = "Config")
	int32 GetTokenExpirationDays() const { return TokenExpirationDays; }

	/** Set environment (for runtime switching) */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetUseDevEnvironment(bool bUseDev);

private:
	/** Dev API base URL */
	UPROPERTY()
	FString ApiBaseUrlDev = TEXT("http://127.0.0.1:8000/api");

	/** Prod API base URL */
	UPROPERTY()
	FString ApiBaseUrlProd = TEXT("https://api.sewerscuttle.com/api");

	/** Use dev environment */
	UPROPERTY()
	bool bUseDevEnvironment = true;

	/** Token expiration in days */
	UPROPERTY()
	int32 TokenExpirationDays = 7;

	/** Singleton instance */
	static UConfigManager* Instance;

	/** Load configuration from INI file */
	void LoadFromConfig();
};

