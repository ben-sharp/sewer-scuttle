// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SecureStorage.generated.h"

UCLASS()
class SEWERSCUTTLE_API USecureStorage : public UObject
{
	GENERATED_BODY()

public:
	static USecureStorage* Get();

	void Save(const FString& Key, const FString& Value);
	FString Load(const FString& Key) const;
	void Remove(const FString& Key);

private:
	FString GetStoragePath() const;
};

