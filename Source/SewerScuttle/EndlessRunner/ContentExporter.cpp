// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExporter.h"
#include "ContentRegistry.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UContentExporter::UContentExporter()
{
	ContentRegistry = nullptr;
}

FString UContentExporter::ExportToJSON() const
{
	if (!ContentRegistry)
	{
		return TEXT("");
	}

	FString Json;
	Json += TEXT("{\n");
	Json += FString::Printf(TEXT("  \"version\": \"%s\",\n"), *ContentRegistry->GetContentVersion());
	Json += FString::Printf(TEXT("  \"exported_at\": \"%s\",\n"), *FDateTime::Now().ToIso8601());
	Json += TEXT("  \"definitions\": [\n");

	TArray<FContentDefinition> AllContent = ContentRegistry->GetAllContent();
	for (int32 i = 0; i < AllContent.Num(); i++)
	{
		const FContentDefinition& Def = AllContent[i];
		if (i > 0) Json += TEXT(",\n");

		Json += TEXT("    {\n");
		Json += FString::Printf(TEXT("      \"type\": \"%s\",\n"), *Def.Type);
		Json += FString::Printf(TEXT("      \"id\": \"%s\",\n"), *Def.Id);
		Json += FString::Printf(TEXT("      \"name\": \"%s\",\n"), *Def.Name);
		Json += TEXT("      \"properties\": {\n");

		int32 PropIndex = 0;
		for (const auto& Pair : Def.Properties)
		{
			if (PropIndex > 0) Json += TEXT(",\n");
			Json += FString::Printf(TEXT("        \"%s\": "), *Pair.Key);
			
			// Try to parse as number, otherwise treat as string
			if (Pair.Value.IsNumeric())
			{
				Json += Pair.Value;
			}
			else if (Pair.Value == TEXT("true") || Pair.Value == TEXT("false"))
			{
				Json += Pair.Value;
			}
			else if (Pair.Value.StartsWith(TEXT("[")) || Pair.Value.StartsWith(TEXT("{")))
			{
				// Already JSON-like
				Json += Pair.Value;
			}
			else
			{
				Json += FString::Printf(TEXT("\"%s\""), *Pair.Value);
			}
			PropIndex++;
		}

		Json += TEXT("\n      }\n");
		Json += TEXT("    }");
	}

	Json += TEXT("\n  ]\n");
	Json += TEXT("}\n");

	return Json;
}

bool UContentExporter::ExportToFile(const FString& FilePath) const
{
	FString JsonContent = ExportToJSON();
	if (JsonContent.IsEmpty())
	{
		return false;
	}

	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	if (!Directory.IsEmpty())
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	return FFileHelper::SaveStringToFile(JsonContent, *FilePath);
}

bool UContentExporter::ExportToBackend() const
{
	// Get project directory (e.g., E:\PluginProjects\SewerScuttle\)
	FString ProjectDir = FPaths::ProjectDir();
	
	// Backend is in the project directory, not sibling
	// Navigate to Backend/storage/content directory
	FString BackendPath = FPaths::Combine(*ProjectDir, TEXT("Backend"), TEXT("storage"), TEXT("content"));
	
	// Create directory if needed
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*BackendPath);
	
	// Export to latest.json
	FString FilePath = FPaths::Combine(*BackendPath, TEXT("latest.json"));
	
	UE_LOG(LogTemp, Log, TEXT("ContentExporter: Exporting to %s"), *FilePath);
	
	return ExportToFile(FilePath);
}

void UContentExporter::ExportToAPI(const FString& ApiUrl) const
{
	FString JsonContent = ExportToJSON();
	if (JsonContent.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ContentExporter: No content to export"));
		return;
	}

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	FString FullUrl = ApiUrl;
	if (!FullUrl.EndsWith(TEXT("/api/content/import")))
	{
		if (!FullUrl.EndsWith(TEXT("/")))
		{
			FullUrl += TEXT("/");
		}
		FullUrl += TEXT("api/content/import");
	}

	Request->SetURL(FullUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonContent);

	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			int32 ResponseCode = Response->GetResponseCode();
			if (ResponseCode >= 200 && ResponseCode < 300)
			{
				UE_LOG(LogTemp, Log, TEXT("ContentExporter: Successfully exported content to API"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ContentExporter: API returned error code %d"), ResponseCode);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ContentExporter: Failed to export content to API"));
		}
	});

	Request->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("ContentExporter: Exporting content to %s"), *FullUrl);
}

