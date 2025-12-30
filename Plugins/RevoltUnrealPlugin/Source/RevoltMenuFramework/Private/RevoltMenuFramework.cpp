#include "RevoltMenuFramework.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FRevoltMenuFrameworkModule"

// Debug logging function - only logs to UE_LOG for development builds
void FRevoltMenuFrameworkModule::LogToFileAndScreen(const FString& Message, bool bError)
{
	// Only log to UE_LOG for development builds - no on-screen or file logging in cooked builds
	UE_LOG(LogTemp, Log, TEXT("RevoltMenuFramework: %s"), *Message);
}

void FRevoltMenuFrameworkModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FRevoltMenuFrameworkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRevoltMenuFrameworkModule, RevoltMenuFramework)

