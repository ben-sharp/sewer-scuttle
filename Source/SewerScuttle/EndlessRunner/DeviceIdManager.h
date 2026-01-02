// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeviceIdManager.generated.h"

/**
 * Manages unique device identification
 */
UCLASS()
class SEWERSCUTTLE_API UDeviceIdManager : public UObject
{
	GENERATED_BODY()

public:
	static UDeviceIdManager* Get();

	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetDeviceId() const;

	UFUNCTION(BlueprintPure, Category = "Identity")
	bool HasDeviceId() const;

private:
	mutable FString CachedDeviceId;
};

