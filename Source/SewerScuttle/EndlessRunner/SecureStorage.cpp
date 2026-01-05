// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecureStorage.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

USecureStorage* USecureStorage::Get()
{
	return GetMutableDefault<USecureStorage>();
}

FString USecureStorage::GetStoragePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("SewerScuttle/auth.dat");
}

void USecureStorage::Save(const FString& Key, const FString& Value)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString Directory = FPaths::GetPath(GetStoragePath());

	if (!PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	// Simple Base64 encoding for "security" (obfuscation)
	// In a real production app, you'd use platform-specific secure storage (Keychain/EncryptedFile)
	FString Encoded = FBase64::Encode(Value);
	FFileHelper::SaveStringToFile(Encoded, *GetStoragePath());
}

FString USecureStorage::Load(const FString& Key) const
{
	FString Encoded;
	if (FFileHelper::LoadFileToString(Encoded, *GetStoragePath()))
	{
		FString Decoded;
		if (FBase64::Decode(Encoded, Decoded))
		{
			return Decoded;
		}
	}
	return TEXT("");
}

void USecureStorage::Remove(const FString& Key)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*GetStoragePath()))
	{
		PlatformFile.DeleteFile(*GetStoragePath());
	}
}

