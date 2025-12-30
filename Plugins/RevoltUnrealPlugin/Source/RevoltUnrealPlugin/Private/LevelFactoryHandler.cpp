// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelFactoryHandler.h"
#include "Engine/World.h"
#include "Factories/WorldFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "GameFramework/PhysicsVolume.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Editor.h"
#include "Engine/Level.h"
#include "Engine/WorldComposition.h"
#include "GameFramework/WorldSettings.h"

UWorld* FLevelFactoryHandler::CreateLevel(const FString& LevelName, const FString& PackagePath)
{
	if (LevelName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Level name is empty"));
		return nullptr;
	}

	// Generate full package path
	FString FullPackagePath = GenerateLevelPackagePath(PackagePath, LevelName);

	// Check if level already exists
	if (FindLevel(LevelName))
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Level '%s' already exists"), *LevelName);
		return nullptr;
	}

	// Create package
	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create package '%s'"), *FullPackagePath);
		return nullptr;
	}

	// Create world using factory
	UWorldFactory* Factory = NewObject<UWorldFactory>();
	Factory->WorldType = EWorldType::Inactive;
	Factory->bInformEngineOfWorld = false;

	UWorld* NewWorld = Cast<UWorld>(Factory->FactoryCreateNew(
		UWorld::StaticClass(),
		Package,
		*LevelName,
		RF_Public | RF_Standalone,
		nullptr,
		GWarn
	));

	if (!NewWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create level '%s'"), *LevelName);
		return nullptr;
	}

	// Initialize the world
	NewWorld->WorldType = EWorldType::Editor;

	// Add essential level actors (similar to what Unreal does when creating new levels)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Add WorldSettings (essential for level functionality)
	AWorldSettings* WorldSettings = NewWorld->SpawnActor<AWorldSettings>(AWorldSettings::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (WorldSettings)
	{
		WorldSettings->SetActorLabel(TEXT("WorldSettings"));
	}

	// Add Default Physics Volume
	APhysicsVolume* DefaultPhysicsVolume = NewWorld->SpawnActor<APhysicsVolume>(APhysicsVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (DefaultPhysicsVolume)
	{
		DefaultPhysicsVolume->bAlwaysRelevant = true;
		DefaultPhysicsVolume->Priority = -1000; // Default priority
	}

	// Add Brush (default brush for level editing)
	ABrush* DefaultBrush = NewWorld->SpawnActor<ABrush>(ABrush::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (DefaultBrush)
	{
		DefaultBrush->Brush = NewObject<UModel>(DefaultBrush);
		DefaultBrush->Brush->Initialize(nullptr, true);
		DefaultBrush->BrushBuilder = nullptr;
		DefaultBrush->SetActorLabel(TEXT("Brush"));
		DefaultBrush->SetActorHiddenInGame(true);
		DefaultBrush->SetIsTemporarilyHiddenInEditor(true);
	}

	// Note: WorldDataLayers are typically created automatically by the engine when needed

	// Mark package dirty and save
	Package->MarkPackageDirty();
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPackagePath, FPackageName::GetMapPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	if (UPackage::SavePackage(Package, NewWorld, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully created level '%s' at '%s'"), *LevelName, *FullPackagePath);
		
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(NewWorld);
		
		return NewWorld;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save new level '%s'"), *LevelName);
	return nullptr;
}

UWorld* FLevelFactoryHandler::DuplicateLevel(UWorld* SourceLevel, const FString& NewName, const FString& NewPath)
{
	if (!SourceLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Source level is null"));
		return nullptr;
	}

	if (NewName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: New level name is empty"));
		return nullptr;
	}

	// Check if target level already exists
	if (FindLevel(NewName))
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Level '%s' already exists"), *NewName);
		return nullptr;
	}

	// Determine target path
	FString TargetPath = NewPath;
	if (TargetPath.IsEmpty())
	{
		// Use source level's path
		FString SourcePackageName = SourceLevel->GetOutermost()->GetName();
		TargetPath = FPackageName::GetLongPackagePath(SourcePackageName);
	}

	// Generate full package path
	FString FullPackagePath = GenerateLevelPackagePath(TargetPath, NewName);

	// Create new package
	UPackage* NewPackage = CreatePackage(*FullPackagePath);
	if (!NewPackage)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create package for level duplication"));
		return nullptr;
	}

	// Duplicate the world
	UWorld* DuplicatedWorld = Cast<UWorld>(StaticDuplicateObject(
		SourceLevel,
		NewPackage,
		*NewName
	));

	if (!DuplicatedWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to duplicate level"));
		return nullptr;
	}

	// Set world type to inactive
	DuplicatedWorld->WorldType = EWorldType::Inactive;

	// Mark package dirty and save
	NewPackage->MarkPackageDirty();
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPackagePath, FPackageName::GetMapPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	if (UPackage::SavePackage(NewPackage, DuplicatedWorld, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully duplicated level to '%s'"), *FullPackagePath);
		
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(DuplicatedWorld);
		
		return DuplicatedWorld;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save duplicated level"));
	return nullptr;
}

UWorld* FLevelFactoryHandler::FindLevel(const FString& LevelName)
{
	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for level by name
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetName.ToString().Equals(LevelName, ESearchCase::IgnoreCase) ||
			AssetData.AssetName.ToString().Contains(LevelName, ESearchCase::IgnoreCase))
		{
			UWorld* World = Cast<UWorld>(AssetData.GetAsset());
			if (World)
			{
				return World;
			}
		}
	}

	return nullptr;
}

bool FLevelFactoryHandler::ValidateLevelName(const FString& LevelName, FString& OutError)
{
	if (LevelName.IsEmpty())
	{
		OutError = TEXT("Level name cannot be empty");
		return false;
	}

	// Check if level already exists
	if (FindLevel(LevelName))
	{
		OutError = FString::Printf(TEXT("Level '%s' already exists"), *LevelName);
		return false;
	}

	// Check for invalid characters (basic validation)
	if (LevelName.Contains(TEXT(" ")) ||
		LevelName.Contains(TEXT("\t")) ||
		LevelName.Contains(TEXT("\n")) ||
		LevelName.Contains(TEXT("\r")) ||
		LevelName.Contains(TEXT("!")) ||
		LevelName.Contains(TEXT("@")) ||
		LevelName.Contains(TEXT("#")) ||
		LevelName.Contains(TEXT("$")) ||
		LevelName.Contains(TEXT("%")) ||
		LevelName.Contains(TEXT("^")) ||
		LevelName.Contains(TEXT("&")) ||
		LevelName.Contains(TEXT("*")) ||
		LevelName.Contains(TEXT("(")) ||
		LevelName.Contains(TEXT(")")) ||
		LevelName.Contains(TEXT("+")) ||
		LevelName.Contains(TEXT("=")) ||
		LevelName.Contains(TEXT("[")) ||
		LevelName.Contains(TEXT("]")) ||
		LevelName.Contains(TEXT("{")) ||
		LevelName.Contains(TEXT("}")) ||
		LevelName.Contains(TEXT("|")) ||
		LevelName.Contains(TEXT("\\")) ||
		LevelName.Contains(TEXT(";")) ||
		LevelName.Contains(TEXT(":")) ||
		LevelName.Contains(TEXT("'")) ||
		LevelName.Contains(TEXT("\"")) ||
		LevelName.Contains(TEXT("<")) ||
		LevelName.Contains(TEXT(">")) ||
		LevelName.Contains(TEXT("?")) ||
		LevelName.Contains(TEXT("/")) ||
		LevelName.Contains(TEXT(",")))
	{
		OutError = FString::Printf(TEXT("Level name '%s' contains invalid characters"), *LevelName);
		return false;
	}

	return true;
}

FString FLevelFactoryHandler::GenerateLevelPackagePath(const FString& PackagePath, const FString& LevelName)
{
	FString CleanPath = ValidateAndCleanPath(PackagePath);
	
	// Ensure path ends without trailing slash
	if (CleanPath.EndsWith(TEXT("/")))
	{
		CleanPath = CleanPath.LeftChop(1);
	}

	// Return full path
	return CleanPath / LevelName;
}

FString FLevelFactoryHandler::ValidateAndCleanPath(const FString& PackagePath)
{
	FString CleanPath = PackagePath;

	// Ensure starts with /Game/
	if (!CleanPath.StartsWith(TEXT("/Game/")))
	{
		if (CleanPath.StartsWith(TEXT("/")))
		{
			CleanPath = TEXT("/Game") + CleanPath;
		}
		else
		{
			CleanPath = TEXT("/Game/") + CleanPath;
		}
	}

	return CleanPath;
}

