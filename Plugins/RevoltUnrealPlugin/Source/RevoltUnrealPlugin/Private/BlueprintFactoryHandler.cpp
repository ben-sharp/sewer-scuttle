// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintFactoryHandler.h"
#include "BlueprintEditHandler.h"
#include "Engine/Blueprint.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/IAssetRegistry.h"

UBlueprint* FBlueprintFactoryHandler::CreateBlueprint(UClass* ParentClass, const FString& BlueprintName, const FString& PackagePath)
{
	if (!ParentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Parent class is null"));
		return nullptr;
	}

	if (BlueprintName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Blueprint name is empty"));
		return nullptr;
	}

	// Generate full package path
	FString FullPackagePath = GeneratePackagePath(PackagePath, BlueprintName);

	// Create package
	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create package '%s'"), *FullPackagePath);
		return nullptr;
	}

	// Create blueprint using factory
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClass;

	UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(
		UBlueprint::StaticClass(),
		Package,
		*BlueprintName,
		RF_Public | RF_Standalone,
		nullptr,
		GWarn
	));

	if (!NewBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create blueprint '%s'"), *BlueprintName);
		return nullptr;
	}

	// Compile the new blueprint
	FKismetEditorUtilities::CompileBlueprint(NewBlueprint);

	// Mark package dirty and save
	Package->MarkPackageDirty();
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPackagePath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	if (UPackage::SavePackage(Package, NewBlueprint, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully created blueprint '%s' at '%s'"), *BlueprintName, *FullPackagePath);
		
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(NewBlueprint);
		
		return NewBlueprint;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save new blueprint '%s'"), *BlueprintName);
	return nullptr;
}

UBlueprint* FBlueprintFactoryHandler::DuplicateBlueprint(UBlueprint* SourceBlueprint, const FString& NewName, const FString& NewPath)
{
	if (!SourceBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Source blueprint is null"));
		return nullptr;
	}

	// Determine target path
	FString TargetPath = NewPath;
	if (TargetPath.IsEmpty())
	{
		// Use source blueprint's path
		FString SourcePackageName = SourceBlueprint->GetOutermost()->GetName();
		TargetPath = FPackageName::GetLongPackagePath(SourcePackageName);
	}

	// Generate full package path
	FString FullPackagePath = GeneratePackagePath(TargetPath, NewName);

	// Create new package
	UPackage* NewPackage = CreatePackage(*FullPackagePath);
	if (!NewPackage)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create package for duplication"));
		return nullptr;
	}

	// Duplicate the blueprint
	UBlueprint* DuplicatedBlueprint = Cast<UBlueprint>(StaticDuplicateObject(
		SourceBlueprint,
		NewPackage,
		*NewName
	));

	if (!DuplicatedBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to duplicate blueprint"));
		return nullptr;
	}

	// Compile the duplicated blueprint
	FKismetEditorUtilities::CompileBlueprint(DuplicatedBlueprint);

	// Mark package dirty and save
	NewPackage->MarkPackageDirty();
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPackagePath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	if (UPackage::SavePackage(NewPackage, DuplicatedBlueprint, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully duplicated blueprint to '%s'"), *FullPackagePath);
		
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(DuplicatedBlueprint);
		
		return DuplicatedBlueprint;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save duplicated blueprint"));
	return nullptr;
}

UClass* FBlueprintFactoryHandler::FindClass(const FString& ClassName)
{
	// Try to find as UClass
	UClass* FoundClass = FindObject<UClass>(nullptr, *ClassName);
	if (FoundClass)
	{
		return FoundClass;
	}

	// Try to find blueprint and get its generated class
	UBlueprint* Blueprint = FBlueprintEditHandler::FindBlueprint(ClassName);
	if (Blueprint && Blueprint->GeneratedClass)
	{
		return Blueprint->GeneratedClass;
	}

	return nullptr;
}

FString FBlueprintFactoryHandler::GeneratePackagePath(const FString& PackagePath, const FString& BlueprintName)
{
	FString CleanPath = ValidateAndCleanPath(PackagePath);
	
	// Ensure path ends without trailing slash
	if (CleanPath.EndsWith(TEXT("/")))
	{
		CleanPath = CleanPath.LeftChop(1);
	}

	// Return full path
	return CleanPath / BlueprintName;
}

FString FBlueprintFactoryHandler::ValidateAndCleanPath(const FString& PackagePath)
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

