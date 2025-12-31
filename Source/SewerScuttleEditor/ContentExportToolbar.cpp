// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentExportToolbar.h"
#include "EndlessRunner/ContentExportLibrary.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LevelEditor.h"
#include "Styling/AppStyle.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ToolMenus.h"
#include "LevelEditorMenuContext.h"

#define LOCTEXT_NAMESPACE "FContentExportToolbarModule"

class FContentExportToolbarCommands : public TCommands<FContentExportToolbarCommands>
{
public:
	FContentExportToolbarCommands()
		: TCommands<FContentExportToolbarCommands>(
			TEXT("ContentExportToolbar"),
			NSLOCTEXT("Contexts", "ContentExportToolbar", "Content Export Toolbar"),
			NAME_None,
			FAppStyle::GetAppStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(ExportContentAction, "Export Content to Backend", "Exports all content definitions to Backend/storage/content/latest.json", EUserInterfaceActionType::Button, FInputChord());
	}

	TSharedPtr<FUICommandInfo> ExportContentAction;
};

void FContentExportToolbarModule::StartupModule()
{
	// Register commands
	FContentExportToolbarCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FContentExportToolbarCommands::Get().ExportContentAction,
		FExecuteAction::CreateRaw(this, &FContentExportToolbarModule::ExportContentToBackend),
		FCanExecuteAction());

	// Add toolbar extension using ToolMenus (UE 5.0+)
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FContentExportToolbarModule::RegisterMenus));
}

void FContentExportToolbarModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FContentExportToolbarCommands::Unregister();
}

void FContentExportToolbarModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	// Try multiple toolbar locations
	{
		// Main Play Toolbar (next to Play button)
		UToolMenu* PlayToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		if (PlayToolBar)
		{
			FToolMenuSection& Section = PlayToolBar->FindOrAddSection("PluginTools");
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				FContentExportToolbarCommands::Get().ExportContentAction
			));
		}
	}

	{
		// Main Level Editor Toolbar
		UToolMenu* MainToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		if (MainToolBar)
		{
			FToolMenuSection& Section = MainToolBar->FindOrAddSection("Settings");
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				FContentExportToolbarCommands::Get().ExportContentAction
			));
		}
	}

	// Also add to Tools menu as backup
	{
		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		if (ToolsMenu)
		{
			FToolMenuSection& Section = ToolsMenu->FindOrAddSection("Programming");
			Section.AddMenuEntryWithCommandList(FContentExportToolbarCommands::Get().ExportContentAction, PluginCommands);
		}
	}
}

void FContentExportToolbarModule::ExportContentToBackend()
{
	// Get version from user or use default
	FString Version = TEXT("1.0.0");
	
	// Try to export
	bool bSuccess = UContentExportLibrary::ExportContentToBackend(Version);
	
	if (bSuccess)
	{
		FString Message = FString::Printf(TEXT("Content version %s exported successfully!\n\nFile: Backend/storage/content/latest.json\n\nYou can now import it using:\nphp artisan content:import storage/content/latest.json"), *Version);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, 
			FText::FromString(TEXT("Failed to export content. Check Output Log for details.")));
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FContentExportToolbarModule, SewerScuttleEditor)
