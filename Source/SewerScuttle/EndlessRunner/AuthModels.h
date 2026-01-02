// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthModels.generated.h"

USTRUCT(BlueprintType)
struct FUserCredentials
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Email;

	UPROPERTY(BlueprintReadWrite)
	FString Password;
};

USTRUCT(BlueprintType)
struct FAuthUserData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Email;
};

USTRUCT(BlueprintType)
struct FAuthResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Token;

	UPROPERTY(BlueprintReadWrite)
	FAuthUserData User;
};

