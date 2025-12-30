// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevoltUnrealPlugin.h"
#include "HttpServerManager.h"
#include "RevoltSettings.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FRevoltUnrealPluginModule"

void FRevoltUnrealPluginModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("RevoltUnrealPlugin: Module starting up"));

	// Initialize menus
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRevoltUnrealPluginModule::RegisterMenus));

	// Add console command to start server
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Revolt.StartServer"),
		TEXT("Start the Revolt HTTP API server"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			FHttpServerManager::Get().StartServer();
		})
	);

	// Force start server for testing
	UE_LOG(LogTemp, Log, TEXT("RevoltUnrealPlugin: Force starting server for testing..."));
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimerForNextTick([]()
		{
			FHttpServerManager::Get().StartServer();
		});
	}
	else
	{
		// Fallback if GEditor is not available yet
		FHttpServerManager::Get().StartServer();
	}
}

void FRevoltUnrealPluginModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("RevoltUnrealPlugin: Module shutting down"));

	// Stop the HTTP server
	FHttpServerManager::Get().StopServer();

	// Unregister menus
	UnregisterMenus();
}

void FRevoltUnrealPluginModule::RegisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	UToolMenu* Menu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection("RevoltPlugin", LOCTEXT("RevoltPluginSection", "Revolt Plugin"));
	
	Section.AddMenuEntry(
		"StartServer",
		LOCTEXT("StartServer", "Start Revolt Server"),
		LOCTEXT("StartServerTooltip", "Start the HTTP API server"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FHttpServerManager::Get().StartServer();
		}))
	);

	Section.AddMenuEntry(
		"StopServer",
		LOCTEXT("StopServer", "Stop Revolt Server"),
		LOCTEXT("StopServerTooltip", "Stop the HTTP API server"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FHttpServerManager::Get().StopServer();
		}))
	);

	Section.AddMenuEntry(
		"RestartServer",
		LOCTEXT("RestartServer", "Restart Revolt Server"),
		LOCTEXT("RestartServerTooltip", "Restart the HTTP API server"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FHttpServerManager::Get().StopServer();
			FHttpServerManager::Get().StartServer();
		}))
	);
}

void FRevoltUnrealPluginModule::UnregisterMenus()
{
	UToolMenus::UnregisterOwner(this);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRevoltUnrealPluginModule, RevoltUnrealPlugin)

