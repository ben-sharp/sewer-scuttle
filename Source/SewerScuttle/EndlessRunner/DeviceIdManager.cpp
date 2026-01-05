// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeviceIdManager.h"
#include "Misc/Guid.h"
#include "Misc/CommandLine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ConfigManager.h"

UDeviceIdManager* UDeviceIdManager::Get()
{
	return GetMutableDefault<UDeviceIdManager>();
}

FString UDeviceIdManager::GetDeviceId() const
{
	if (CachedDeviceId.IsEmpty())
	{
		// 1. Try command line override (highest priority)
		FString CommandLineId;
		if (FParse::Value(FCommandLine::Get(), TEXT("DeviceId="), CommandLineId))
		{
			CachedDeviceId = CommandLineId;
		}

		// 2. Try Editor dev override
		if (CachedDeviceId.IsEmpty() && GIsEditor)
		{
			if (UConfigManager* Config = UConfigManager::Get())
			{
				CachedDeviceId = Config->GetDevDeviceId();
			}
		}

		// 3. Standard Logic
		if (CachedDeviceId.IsEmpty())
		{
			CachedDeviceId = UKismetSystemLibrary::GetDeviceId();
			if (CachedDeviceId.IsEmpty())
			{
				CachedDeviceId = FGuid::NewGuid().ToString();
			}
		}
	}
	return CachedDeviceId;
}

bool UDeviceIdManager::HasDeviceId() const
{
	return !GetDeviceId().IsEmpty();
}

