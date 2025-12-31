// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SecureStorage.generated.h"

/**
 * Secure storage for sensitive data (tokens, device IDs)
 * Uses encrypted file storage compatible with Windows and Android
 */
UCLASS()
class SEWERSCUTTLE_API USecureStorage : public UObject
{
	GENERATED_BODY()

public:
	USecureStorage();

	/** Save authentication token */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool SaveToken(const FString& Token);

	/** Load authentication token */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool LoadToken(FString& OutToken);

	/** Clear authentication token */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool ClearToken();

	/** Save device ID */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool SaveDeviceId(const FString& DeviceId);

	/** Load device ID */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool LoadDeviceId(FString& OutDeviceId);

	/** Clear all stored data */
	UFUNCTION(BlueprintCallable, Category = "Secure Storage")
	bool ClearAll();

	/** Check if token exists */
	UFUNCTION(BlueprintPure, Category = "Secure Storage")
	bool HasToken() const;

	/** Check if device ID exists */
	UFUNCTION(BlueprintPure, Category = "Secure Storage")
	bool HasDeviceId() const;

private:
	/** Storage file path */
	FString GetStoragePath() const;

	/** Encrypt data */
	FString Encrypt(const FString& PlainText) const;

	/** Decrypt data */
	FString Decrypt(const FString& CipherText) const;

	/** Generate encryption key from app name */
	FString GetEncryptionKey() const;

	/** Simple XOR encryption (can be upgraded to AES later) */
	FString SimpleXOREncrypt(const FString& Input, const FString& Key) const;
};

