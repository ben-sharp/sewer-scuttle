// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Main module class for the Revolt Unreal Plugin
 * Provides HTTP API server for real-time project querying
 */
class FRevoltUnrealPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	
	/** Registers editor menu commands */
	void RegisterMenus();
	
	/** Unregisters editor menu commands */
	void UnregisterMenus();
};

