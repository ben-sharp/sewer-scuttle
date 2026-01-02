// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConfigManager.h"

UConfigManager* UConfigManager::Get()
{
	return GetMutableDefault<UConfigManager>();
}

FString UConfigManager::GetApiBaseUrl() const
{
	FString Base = bUseDevEnvironment ? ApiBaseUrlDev : ApiBaseUrlProd;
	
	// Remove any existing protocol if user included it
	if (Base.StartsWith(TEXT("http://")))
	{
		Base.RightChopInline(7);
	}
	else if (Base.StartsWith(TEXT("https://")))
	{
		Base.RightChopInline(8);
	}

	// Always prepend based on environment
	return (bUseDevEnvironment ? TEXT("http://") : TEXT("https://")) + Base;
}

