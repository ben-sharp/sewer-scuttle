// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ContentExporter.generated.h"

class UContentRegistry;

/**
 * Content Exporter - Exports content definitions to JSON
 */
UCLASS()
class SEWERSCUTTLE_API UContentExporter : public UObject
{
	GENERATED_BODY()

public:
	UContentExporter();

	/** Export all content to JSON string */
	UFUNCTION(BlueprintCallable, Category = "Content")
	FString ExportToJSON() const;

	/** Export to file */
	UFUNCTION(BlueprintCallable, Category = "Content")
	bool ExportToFile(const FString& FilePath) const;

	/** Export to Backend directory (dev mode) */
	UFUNCTION(BlueprintCallable, Category = "Content")
	bool ExportToBackend() const;

	/** Export to API (production) */
	UFUNCTION(BlueprintCallable, Category = "Content")
	void ExportToAPI(const FString& ApiUrl) const;

	/** Set content registry */
	UFUNCTION(BlueprintCallable, Category = "Content")
	void SetContentRegistry(UContentRegistry* Registry) { ContentRegistry = Registry; }

private:
	UPROPERTY()
	UContentRegistry* ContentRegistry;
};

