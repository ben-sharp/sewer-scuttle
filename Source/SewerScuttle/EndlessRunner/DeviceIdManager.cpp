// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeviceIdManager.h"
#include "Misc/Guid.h"
#include "Kismet/KismetSystemLibrary.h"

UDeviceIdManager* UDeviceIdManager::Get()
{
	return GetMutableDefault<UDeviceIdManager>();
}

FString UDeviceIdManager::GetDeviceId() const
{
	if (CachedDeviceId.IsEmpty())
	{
		CachedDeviceId = UKismetSystemLibrary::GetDeviceId();
		if (CachedDeviceId.IsEmpty())
		{
			CachedDeviceId = FGuid::NewGuid().ToString();
		}
	}
	return CachedDeviceId;
}

bool UDeviceIdManager::HasDeviceId() const
{
	return !GetDeviceId().IsEmpty();
}

