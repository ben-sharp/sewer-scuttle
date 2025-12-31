// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeviceIdManager.generated.h"

class USecureStorage;

/**
 * Manages persistent device identifier generation and storage
 * Platform-specific implementation for Windows and Android
 */
UCLASS()
class SEWERSCUTTLE_API UDeviceIdManager : public UObject
{
	GENERATED_BODY()

public:
	UDeviceIdManager();

	/** Get singleton instance */
	UFUNCTION(BlueprintCallable, Category = "Device ID")
	static UDeviceIdManager* Get();

	/** Initialize device ID manager */
	UFUNCTION(BlueprintCallable, Category = "Device ID")
	void Initialize();

	/** Get device ID (generates if not exists) */
	UFUNCTION(BlueprintPure, Category = "Device ID")
	FString GetDeviceId() const { return DeviceId; }

	/** Check if device ID exists */
	UFUNCTION(BlueprintPure, Category = "Device ID")
	bool HasDeviceId() const { return !DeviceId.IsEmpty(); }

private:
	/** Generate device ID using platform-specific methods */
	FString GenerateDeviceId();

	/** Generate Windows device ID */
	FString GenerateWindowsDeviceId();

	/** Generate Android device ID */
	FString GenerateAndroidDeviceId();

	/** Generate fallback UUID */
	FString GenerateFallbackUUID();

	/** Device ID */
	UPROPERTY()
	FString DeviceId;

	/** Secure storage instance */
	UPROPERTY()
	USecureStorage* SecureStorage;

	/** Singleton instance */
	static UDeviceIdManager* Instance;
};

