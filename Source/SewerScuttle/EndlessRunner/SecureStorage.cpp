// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecureStorage.h"
#include "Misc/Base64.h"

USecureStorage* USecureStorage::Get()
{
	return GetMutableDefault<USecureStorage>();
}

void USecureStorage::Save(const FString& Key, const FString& Value)
{
    // Dummy implementation for now
}

FString USecureStorage::Load(const FString& Key) const
{
    return TEXT("");
}

void USecureStorage::Remove(const FString& Key)
{
}

