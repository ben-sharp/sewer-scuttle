// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ContentExporter.generated.h"

class UContentRegistry;

/**
 * Handles exporting game content to JSON for the backend
 */
UCLASS()
class SEWERSCUTTLE_API UContentExporter : public UObject
{
	GENERATED_BODY()

public:
	/** Export registry content to a JSON string */
	FString ExportToJson(UContentRegistry* Registry, const FString& Version);

	/** Export registry content to a file */
	bool ExportToFile(UContentRegistry* Registry, const FString& Version, const FString& FilePath);
};

