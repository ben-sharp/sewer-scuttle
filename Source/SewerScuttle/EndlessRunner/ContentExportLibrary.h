// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContentExportLibrary.generated.h"

/**
 * Blueprint library for exporting game content
 */
UCLASS()
class SEWERSCUTTLE_API UContentExportLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Run the full content export process */
	UFUNCTION(BlueprintCallable, Category = "Content Export")
	static void ExportGameContent(const FString& Version, const FString& FilePath);
};

