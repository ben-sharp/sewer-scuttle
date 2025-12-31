// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExportLibrary.h"
#include "ContentRegistry.h"
#include "ContentExporter.h"

bool UContentExportLibrary::ExportContentToBackend(const FString& Version)
{
	UContentRegistry* Registry = NewObject<UContentRegistry>();
	UContentExporter* Exporter = NewObject<UContentExporter>();

	if (!Registry || !Exporter)
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExportLibrary: Failed to create Content Registry or Exporter"));
		return false;
	}

	Exporter->SetContentRegistry(Registry);
	Registry->SetContentVersion(Version);
	Registry->CollectAllContent();

	bool bSuccess = Exporter->ExportToBackend();
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("ContentExportLibrary: Content version %s exported successfully to Backend/storage/content/latest.json"), *Version);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExportLibrary: Failed to export content"));
	}

	return bSuccess;
}

bool UContentExportLibrary::ExportContentToFile(const FString& FilePath, const FString& Version)
{
	UContentRegistry* Registry = NewObject<UContentRegistry>();
	UContentExporter* Exporter = NewObject<UContentExporter>();

	if (!Registry || !Exporter)
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExportLibrary: Failed to create Content Registry or Exporter"));
		return false;
	}

	Exporter->SetContentRegistry(Registry);
	Registry->SetContentVersion(Version);
	Registry->CollectAllContent();

	bool bSuccess = Exporter->ExportToFile(FilePath);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("ContentExportLibrary: Content version %s exported successfully to %s"), *Version, *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExportLibrary: Failed to export content to %s"), *FilePath);
	}

	return bSuccess;
}

void UContentExportLibrary::ExportContentToAPI(const FString& ApiUrl, const FString& Version)
{
	UContentRegistry* Registry = NewObject<UContentRegistry>();
	UContentExporter* Exporter = NewObject<UContentExporter>();

	if (!Registry || !Exporter)
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExportLibrary: Failed to create Content Registry or Exporter"));
		return;
	}

	Exporter->SetContentRegistry(Registry);
	Registry->SetContentVersion(Version);
	Registry->CollectAllContent();

	Exporter->ExportToAPI(ApiUrl);
	UE_LOG(LogTemp, Log, TEXT("ContentExportLibrary: Content version %s export request sent to %s"), *Version, *ApiUrl);
}

