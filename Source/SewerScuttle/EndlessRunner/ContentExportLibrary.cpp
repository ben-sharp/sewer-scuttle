// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExportLibrary.h"
#include "ContentRegistry.h"
#include "ContentExporter.h"
#include "Misc/MessageDialog.h"

void UContentExportLibrary::ExportGameContent(const FString& Version, const FString& FilePath)
{
	UContentRegistry* Registry = NewObject<UContentRegistry>();
	Registry->GatherContent();

	UContentExporter* Exporter = NewObject<UContentExporter>();
	if (Exporter->ExportToFile(Registry, Version, FilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Successfully exported content to: %s"), *FilePath)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to export content!")));
	}
}

