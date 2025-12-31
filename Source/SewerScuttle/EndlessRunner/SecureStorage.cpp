// Copyright Epic Games, Inc. All Rights Reserved.

#include "SecureStorage.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Base64.h"

USecureStorage::USecureStorage()
{
}

FString USecureStorage::GetStoragePath() const
{
	FString SavedDir = FPaths::ProjectSavedDir();
	FString AuthDir = SavedDir / TEXT("SewerScuttle");
	
	// Create directory if it doesn't exist
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*AuthDir))
	{
		PlatformFile.CreateDirectoryTree(*AuthDir);
	}
	
	return AuthDir / TEXT("auth.dat");
}

FString USecureStorage::GetEncryptionKey() const
{
	// Generate key from app name + fixed salt
	FString AppName = FApp::GetProjectName();
	FString Salt = TEXT("SewerScuttleAuth2024");
	return AppName + Salt;
}

FString USecureStorage::SimpleXOREncrypt(const FString& Input, const FString& Key) const
{
	FString Result;
	Result.Reserve(Input.Len());
	
	const TCHAR* InputChars = *Input;
	const TCHAR* KeyChars = *Key;
	int32 KeyLen = Key.Len();
	
	if (KeyLen == 0)
	{
		return Input; // No encryption if no key
	}
	
	for (int32 i = 0; i < Input.Len(); ++i)
	{
		TCHAR EncryptedChar = InputChars[i] ^ KeyChars[i % KeyLen];
		Result += EncryptedChar;
	}
	
	return Result;
}

FString USecureStorage::Encrypt(const FString& PlainText) const
{
	if (PlainText.IsEmpty())
	{
		return PlainText;
	}
	
	FString Key = GetEncryptionKey();
	FString Encrypted = SimpleXOREncrypt(PlainText, Key);
	
	// Base64 encode for safe storage
	FString Base64Encoded = FBase64::Encode(Encrypted);
	
	return Base64Encoded;
}

FString USecureStorage::Decrypt(const FString& CipherText) const
{
	if (CipherText.IsEmpty())
	{
		return CipherText;
	}
	
	// Base64 decode
	FString Base64Decoded;
	if (!FBase64::Decode(CipherText, Base64Decoded))
	{
		UE_LOG(LogTemp, Warning, TEXT("SecureStorage: Failed to decode base64"));
		return FString();
	}
	
	FString Key = GetEncryptionKey();
	FString Decrypted = SimpleXOREncrypt(Base64Decoded, Key);
	
	return Decrypted;
}

bool USecureStorage::SaveToken(const FString& Token)
{
	if (Token.IsEmpty())
	{
		return ClearToken();
	}
	
	FString EncryptedToken = Encrypt(Token);
	FString StoragePath = GetStoragePath();
	
	// Read existing data
	TMap<FString, FString> Data;
	FString ExistingContent;
	if (FFileHelper::LoadFileToString(ExistingContent, *StoragePath))
	{
		// Parse existing data (simple key=value format, one per line)
		TArray<FString> Lines;
		ExistingContent.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			FString Key, Value;
			if (Line.Split(TEXT("="), &Key, &Value))
			{
				Data.Add(Key, Value);
			}
		}
	}
	
	// Update token
	Data.Add(TEXT("token"), EncryptedToken);
	
	// Write back
	FString Output;
	for (const auto& Pair : Data)
	{
		Output += Pair.Key + TEXT("=") + Pair.Value + TEXT("\n");
	}
	
	bool bSuccess = FFileHelper::SaveStringToFile(Output, *StoragePath);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("SecureStorage: Token saved successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SecureStorage: Failed to save token"));
	}
	
	return bSuccess;
}

bool USecureStorage::LoadToken(FString& OutToken)
{
	FString StoragePath = GetStoragePath();
	
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *StoragePath))
	{
		return false;
	}
	
	// Parse data
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (Line.Split(TEXT("="), &Key, &Value) && Key == TEXT("token"))
		{
			OutToken = Decrypt(Value);
			return !OutToken.IsEmpty();
		}
	}
	
	return false;
}

bool USecureStorage::ClearToken()
{
	FString StoragePath = GetStoragePath();
	
	// Read existing data
	TMap<FString, FString> Data;
	FString ExistingContent;
	if (FFileHelper::LoadFileToString(ExistingContent, *StoragePath))
	{
		TArray<FString> Lines;
		ExistingContent.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			FString Key, Value;
			if (Line.Split(TEXT("="), &Key, &Value) && Key != TEXT("token"))
			{
				Data.Add(Key, Value);
			}
		}
	}
	
	// Write back without token
	FString Output;
	for (const auto& Pair : Data)
	{
		Output += Pair.Key + TEXT("=") + Pair.Value + TEXT("\n");
	}
	
	return FFileHelper::SaveStringToFile(Output, *StoragePath);
}

bool USecureStorage::SaveDeviceId(const FString& DeviceId)
{
	if (DeviceId.IsEmpty())
	{
		return false;
	}
	
	FString EncryptedDeviceId = Encrypt(DeviceId);
	FString StoragePath = GetStoragePath();
	
	// Read existing data
	TMap<FString, FString> Data;
	FString ExistingContent;
	if (FFileHelper::LoadFileToString(ExistingContent, *StoragePath))
	{
		TArray<FString> Lines;
		ExistingContent.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			FString Key, Value;
			if (Line.Split(TEXT("="), &Key, &Value))
			{
				Data.Add(Key, Value);
			}
		}
	}
	
	// Update device ID
	Data.Add(TEXT("device_id"), EncryptedDeviceId);
	
	// Write back
	FString Output;
	for (const auto& Pair : Data)
	{
		Output += Pair.Key + TEXT("=") + Pair.Value + TEXT("\n");
	}
	
	bool bSuccess = FFileHelper::SaveStringToFile(Output, *StoragePath);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("SecureStorage: Device ID saved successfully"));
	}
	
	return bSuccess;
}

bool USecureStorage::LoadDeviceId(FString& OutDeviceId)
{
	FString StoragePath = GetStoragePath();
	
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *StoragePath))
	{
		return false;
	}
	
	// Parse data
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (Line.Split(TEXT("="), &Key, &Value) && Key == TEXT("device_id"))
		{
			OutDeviceId = Decrypt(Value);
			return !OutDeviceId.IsEmpty();
		}
	}
	
	return false;
}

bool USecureStorage::ClearAll()
{
	FString StoragePath = GetStoragePath();
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*StoragePath))
	{
		return PlatformFile.DeleteFile(*StoragePath);
	}
	
	return true;
}

bool USecureStorage::HasToken() const
{
	FString StoragePath = const_cast<USecureStorage*>(this)->GetStoragePath();
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *StoragePath))
	{
		return false;
	}
	
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (Line.Split(TEXT("="), &Key, &Value) && Key == TEXT("token"))
		{
			return true;
		}
	}
	return false;
}

bool USecureStorage::HasDeviceId() const
{
	FString StoragePath = const_cast<USecureStorage*>(this)->GetStoragePath();
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *StoragePath))
	{
		return false;
	}
	
	TArray<FString> Lines;
	Content.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Key, Value;
		if (Line.Split(TEXT("="), &Key, &Value) && Key == TEXT("device_id"))
		{
			return true;
		}
	}
	return false;
}

