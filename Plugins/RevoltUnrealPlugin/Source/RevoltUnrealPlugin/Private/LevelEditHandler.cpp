// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelEditHandler.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor.h"

AActor* FLevelEditHandler::SpawnActorInLevel(UWorld* World, UClass* ActorClass, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Cannot spawn actor - World is null"));
		return nullptr;
	}

	if (!ActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Cannot spawn actor - ActorClass is null"));
		return nullptr;
	}

	// Set spawn transform
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Location);
	SpawnTransform.SetRotation(Rotation.Quaternion());
	SpawnTransform.SetScale3D(Scale);

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the actor
	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

	if (SpawnedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Spawned actor '%s' at location (%.2f, %.2f, %.2f)"),
			*SpawnedActor->GetName(), Location.X, Location.Y, Location.Z);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to spawn actor of class '%s'"), *ActorClass->GetName());
	}

	return SpawnedActor;
}

bool FLevelEditHandler::ConfigureActorProperties(AActor* Actor, const TMap<FString, FString>& Properties)
{
	if (!Actor)
	{
		return false;
	}

	bool bAllSucceeded = true;

	for (const auto& PropertyPair : Properties)
	{
		FProperty* Property = Actor->GetClass()->FindPropertyByName(*PropertyPair.Key);
		if (!Property)
		{
			UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Property '%s' not found on actor '%s'"),
				*PropertyPair.Key, *Actor->GetName());
			bAllSucceeded = false;
			continue;
		}

		if (!SetActorPropertyValue(Actor, Property, PropertyPair.Value))
		{
			UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Failed to set property '%s' on actor '%s'"),
				*PropertyPair.Key, *Actor->GetName());
			bAllSucceeded = false;
		}
	}

	return bAllSucceeded;
}

void FLevelEditHandler::AddActorTags(AActor* Actor, const TArray<FString>& Tags)
{
	if (!Actor)
	{
		return;
	}

	for (const FString& Tag : Tags)
	{
		Actor->Tags.AddUnique(FName(*Tag));
	}
}

AActor* FLevelEditHandler::FindActorByName(UWorld* World, const FString& ActorName)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && (Actor->GetName().Equals(ActorName, ESearchCase::IgnoreCase) ||
			Actor->GetActorLabel().Equals(ActorName, ESearchCase::IgnoreCase)))
		{
			return Actor;
		}
	}

	return nullptr;
}

bool FLevelEditHandler::DeleteActor(UWorld* World, AActor* Actor)
{
	if (!World || !Actor)
	{
		return false;
	}

	bool bDestroyed = World->DestroyActor(Actor);
	
	if (bDestroyed)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Deleted actor '%s'"), *Actor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to delete actor '%s'"), *Actor->GetName());
	}

	return bDestroyed;
}

bool FLevelEditHandler::SaveLevel(UWorld* World)
{
	if (!World)
	{
		return false;
	}

	UPackage* Package = World->GetOutermost();
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Could not get package for world"));
		return false;
	}

	// Mark package dirty
	Package->MarkPackageDirty();

	// Save the package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetMapPackageExtension());
	
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	bool bSaved = UPackage::SavePackage(Package, World, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Saved level '%s'"), *World->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save level '%s'"), *World->GetName());
	}

	return bSaved;
}

UWorld* FLevelEditHandler::LoadLevelForEditing(const FString& LevelName)
{
	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Find the level asset
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), AssetDataList, true);

	FString TargetPackagePath;
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetName.ToString().Equals(LevelName, ESearchCase::IgnoreCase) ||
			AssetData.AssetName.ToString().Contains(LevelName, ESearchCase::IgnoreCase))
		{
			TargetPackagePath = AssetData.PackageName.ToString();
			break;
		}
	}

	if (TargetPackagePath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Could not find level '%s'"), *LevelName);
		return nullptr;
	}

	// Load the level
	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Loading level '%s' for editing"), *LevelName);
	
	UWorld* World = Cast<UWorld>(StaticLoadObject(UWorld::StaticClass(), nullptr, *TargetPackagePath));
	
	return World;
}

bool FLevelEditHandler::SetActorPropertyValue(AActor* Actor, FProperty* Property, const FString& Value)
{
	if (!Actor || !Property)
	{
		return false;
	}

	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Actor);
	if (!ValuePtr)
	{
		return false;
	}

	// Import the text value into the property
	const TCHAR* ImportText = *Value;
	Property->ImportText_Direct(ImportText, ValuePtr, Actor, PPF_None);

	return true;
}

