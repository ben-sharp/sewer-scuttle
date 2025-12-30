#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRevoltMenuFrameworkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Debug logging that works in cooked builds */
	static void LogToFileAndScreen(const FString& Message, bool bError = false);
};

