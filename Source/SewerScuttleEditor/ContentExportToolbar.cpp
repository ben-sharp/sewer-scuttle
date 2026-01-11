// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExportToolbar.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "EndlessRunner/ContentExportLibrary.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

IMPLEMENT_MODULE(FContentExportToolbarModule, SewerScuttleEditor)

void FContentExportToolbarModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("SewerScuttleEditor Module Starting Up..."));
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FContentExportToolbarModule::RegisterMenus));
}

void FContentExportToolbarModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FContentExportToolbarModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->FindOrAddSection("PluginOperations");

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"ExportContent",
		FExecuteAction::CreateRaw(this, &FContentExportToolbarModule::OnExportButtonClicked),
		FText::FromString(TEXT("Export JSON")),
		FText::FromString(TEXT("Export game content definitions to JSON for the backend")),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details")
	));
}

void FContentExportToolbarModule::OnExportButtonClicked()
{
	// Default paths
	FString ProjectDir = FPaths::ProjectDir();
	FString DefaultPath = ProjectDir / TEXT("Backend/storage/app/content/latest.json");
	
	UContentExportLibrary::ExportGameContent(TEXT("1.0.0"), DefaultPath);
}

