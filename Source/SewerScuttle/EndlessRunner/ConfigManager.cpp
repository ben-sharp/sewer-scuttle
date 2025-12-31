// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConfigManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"

UConfigManager* UConfigManager::Instance = nullptr;

UConfigManager* UConfigManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UConfigManager>();
		Instance->AddToRoot(); // Prevent garbage collection
		Instance->Initialize();
	}
	return Instance;
}

void UConfigManager::Initialize()
{
	LoadFromConfig();
}

void UConfigManager::SetUseDevEnvironment(bool bUseDev)
{
	bUseDevEnvironment = bUseDev;
	UE_LOG(LogTemp, Log, TEXT("ConfigManager: Environment set to %s"), bUseDev ? TEXT("DEV") : TEXT("PROD"));
}

void UConfigManager::LoadFromConfig()
{
	// Read from DefaultGame.ini
	const FString ConfigSection = TEXT("SewerScuttle");
	
	// Read API URLs - use default values if not found
	FString ReadDevUrl = ApiBaseUrlDev; // Use default as fallback
	FString ReadProdUrl = ApiBaseUrlProd; // Use default as fallback
	
	bool bDevUrlFound = GConfig->GetString(*ConfigSection, TEXT("ApiBaseUrl"), ReadDevUrl, GGameIni);
	bool bProdUrlFound = GConfig->GetString(*ConfigSection, TEXT("ApiBaseUrlProd"), ReadProdUrl, GGameIni);
	
	UE_LOG(LogTemp, Warning, TEXT("ConfigManager: Reading config from section '%s'"), *ConfigSection);
	UE_LOG(LogTemp, Warning, TEXT("ConfigManager: ApiBaseUrl found: %d, value: '%s'"), bDevUrlFound ? 1 : 0, *ReadDevUrl);
	UE_LOG(LogTemp, Warning, TEXT("ConfigManager: ApiBaseUrlProd found: %d, value: '%s'"), bProdUrlFound ? 1 : 0, *ReadProdUrl);
	
	// Prepend protocol if not present
	if (!ReadDevUrl.StartsWith(TEXT("http://")) && !ReadDevUrl.StartsWith(TEXT("https://")))
	{
		ReadDevUrl = TEXT("http://") + ReadDevUrl;
	}
	if (!ReadProdUrl.StartsWith(TEXT("http://")) && !ReadProdUrl.StartsWith(TEXT("https://")))
	{
		ReadProdUrl = TEXT("https://") + ReadProdUrl;
	}
	
	ApiBaseUrlDev = ReadDevUrl;
	ApiBaseUrlProd = ReadProdUrl;
	
	// Read environment flag
	bool bConfigUseDev = true;
	GConfig->GetBool(*ConfigSection, TEXT("bUseDevEnvironment"), bConfigUseDev, GGameIni);
	bUseDevEnvironment = bConfigUseDev;
	
	// Read token expiration
	int32 ConfigExpirationDays = 7;
	GConfig->GetInt(*ConfigSection, TEXT("TokenExpirationDays"), ConfigExpirationDays, GGameIni);
	TokenExpirationDays = ConfigExpirationDays;

	// Check for command line override
	FString CommandLine = FCommandLine::Get();
	if (CommandLine.Contains(TEXT("-dev"), ESearchCase::IgnoreCase))
	{
		bUseDevEnvironment = true;
	}
	else if (CommandLine.Contains(TEXT("-prod"), ESearchCase::IgnoreCase))
	{
		bUseDevEnvironment = false;
	}

	UE_LOG(LogTemp, Warning, TEXT("ConfigManager: Loaded config - Environment: %s, Dev URL: '%s', Prod URL: '%s', Token Expiration: %d days"),
		bUseDevEnvironment ? TEXT("DEV") : TEXT("PROD"), *ApiBaseUrlDev, *ApiBaseUrlProd, TokenExpirationDays);
	
	FString CurrentBaseUrl = bUseDevEnvironment ? ApiBaseUrlDev : ApiBaseUrlProd;
	UE_LOG(LogTemp, Warning, TEXT("ConfigManager: Current BaseUrl: '%s'"), *CurrentBaseUrl);
}

