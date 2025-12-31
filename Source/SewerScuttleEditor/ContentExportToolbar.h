// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

class FContentExportToolbarModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register menus */
	void RegisterMenus();

	/** Export content to backend */
	void ExportContentToBackend();

	/** Command list for toolbar actions */
	TSharedPtr<FUICommandList> PluginCommands;
};

