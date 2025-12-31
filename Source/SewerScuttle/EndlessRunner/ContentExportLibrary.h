// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContentExportLibrary.generated.h"

/**
 * Blueprint Function Library for exporting content to backend
 * Can be called from any Blueprint or C++ code
 */
UCLASS()
class SEWERSCUTTLE_API UContentExportLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Export all content definitions to Backend/storage/content/latest.json
	 * @param Version - Content version string (e.g., "1.0.0")
	 * @return True if export succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export", meta = (CallInEditor = "true"))
	static bool ExportContentToBackend(const FString& Version = TEXT("1.0.0"));

	/**
	 * Export content to a custom file path
	 * @param FilePath - Full path to export file
	 * @param Version - Content version string
	 * @return True if export succeeded, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export", meta = (CallInEditor = "true"))
	static bool ExportContentToFile(const FString& FilePath, const FString& Version = TEXT("1.0.0"));

	/**
	 * Export content to API endpoint
	 * @param ApiUrl - Full API URL (e.g., "http://127.0.0.1:8000/api/content/import")
	 * @param Version - Content version string
	 */
	UFUNCTION(BlueprintCallable, Category = "Content Export")
	static void ExportContentToAPI(const FString& ApiUrl, const FString& Version = TEXT("1.0.0"));
};

