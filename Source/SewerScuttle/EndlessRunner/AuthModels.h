// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AuthModels.generated.h"

/**
 * Authentication request data
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FAuthRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Email;

	UPROPERTY(BlueprintReadWrite)
	FString Password;
};

/**
 * Registration request data
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FRegisterRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Email;

	UPROPERTY(BlueprintReadWrite)
	FString Password;

	UPROPERTY(BlueprintReadWrite)
	FString Username;

	UPROPERTY(BlueprintReadWrite)
	FString DisplayName;
};

/**
 * Device authentication request data
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FDeviceAuthRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString DeviceId;

	UPROPERTY(BlueprintReadWrite)
	FString Username;

	UPROPERTY(BlueprintReadWrite)
	FString DisplayName;
};

/**
 * Player data structure
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Username;

	UPROPERTY(BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, int32> Currency;
};

/**
 * User data structure (renamed to avoid conflict with Unreal's FUserData in PhysicsCore)
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FAuthUserData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Email;

	UPROPERTY(BlueprintReadWrite)
	FPlayerData Player;
};

/**
 * Authentication response data
 */
USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FAuthResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FAuthUserData User;

	UPROPERTY(BlueprintReadWrite)
	FString Token;
};

