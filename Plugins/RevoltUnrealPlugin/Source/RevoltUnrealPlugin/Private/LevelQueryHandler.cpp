// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelQueryHandler.h"
#include "RevoltSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

TSharedPtr<FJsonObject> FLevelQueryHandler::GetAllLevels()
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> LevelsArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Query all level assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), AssetDataList, true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d levels"), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		TSharedPtr<FJsonObject> LevelJson = MakeShared<FJsonObject>();
		LevelJson->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		LevelJson->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
		LevelJson->SetStringField(TEXT("package_path"), AssetData.PackageName.ToString());

		LevelsArray.Add(MakeShared<FJsonValueObject>(LevelJson));
	}

	Result->SetArrayField(TEXT("levels"), LevelsArray);
	Result->SetNumberField(TEXT("count"), LevelsArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FLevelQueryHandler::GetLevelActors(const FString& LevelName, const FQueryOptions& Options)
{
	// Load the level
	UWorld* World = LoadLevel(LevelName);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to load level '%s'"), *LevelName);
		return nullptr;
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ActorsArray;

	// Get max actors from settings
	const URevoltSettings* Settings = GetDefault<URevoltSettings>();
	int32 MaxActors = Settings ? Settings->MaxActorsPerQuery : 1000;

	// Iterate through all actors in the world
	int32 ActorCount = 0;
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor && IsValid(Actor))
		{
			TSharedPtr<FJsonObject> ActorJson = ExtractActorInfo(Actor, Options);
			if (ActorJson)
			{
				ActorsArray.Add(MakeShared<FJsonValueObject>(ActorJson));
				ActorCount++;

				// Limit number of actors
				if (ActorCount >= MaxActors)
				{
					UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Reached max actor limit (%d), stopping enumeration"), MaxActors);
					break;
				}
			}
		}
	}

	Result->SetStringField(TEXT("level_name"), LevelName);
	Result->SetArrayField(TEXT("actors"), ActorsArray);
	Result->SetNumberField(TEXT("count"), ActorsArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

UWorld* FLevelQueryHandler::LoadLevel(const FString& LevelName)
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
	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Loading level '%s' from '%s'"), *LevelName, *TargetPackagePath);
	
	// Use the editor's level loading system
	UWorld* World = Cast<UWorld>(StaticLoadObject(UWorld::StaticClass(), nullptr, *TargetPackagePath));
	
	return World;
}

TSharedPtr<FJsonObject> FLevelQueryHandler::ExtractActorInfo(AActor* Actor, const FQueryOptions& Options)
{
	if (!Actor)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> ActorJson = MakeShared<FJsonObject>();

	// Basic info
	ActorJson->SetStringField(TEXT("name"), Actor->GetName());
	ActorJson->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
	ActorJson->SetStringField(TEXT("label"), Actor->GetActorLabel());

	// Location
	FVector Location = Actor->GetActorLocation();
	TSharedPtr<FJsonObject> LocationJson = MakeShared<FJsonObject>();
	LocationJson->SetNumberField(TEXT("x"), Location.X);
	LocationJson->SetNumberField(TEXT("y"), Location.Y);
	LocationJson->SetNumberField(TEXT("z"), Location.Z);
	ActorJson->SetObjectField(TEXT("location"), LocationJson);

	// Rotation
	FRotator Rotation = Actor->GetActorRotation();
	TSharedPtr<FJsonObject> RotationJson = MakeShared<FJsonObject>();
	RotationJson->SetNumberField(TEXT("pitch"), Rotation.Pitch);
	RotationJson->SetNumberField(TEXT("yaw"), Rotation.Yaw);
	RotationJson->SetNumberField(TEXT("roll"), Rotation.Roll);
	ActorJson->SetObjectField(TEXT("rotation"), RotationJson);

	// Scale
	FVector Scale = Actor->GetActorScale3D();
	TSharedPtr<FJsonObject> ScaleJson = MakeShared<FJsonObject>();
	ScaleJson->SetNumberField(TEXT("x"), Scale.X);
	ScaleJson->SetNumberField(TEXT("y"), Scale.Y);
	ScaleJson->SetNumberField(TEXT("z"), Scale.Z);
	ActorJson->SetObjectField(TEXT("scale"), ScaleJson);

	// Tags
	TArray<TSharedPtr<FJsonValue>> TagsArray;
	for (const FName& Tag : Actor->Tags)
	{
		TagsArray.Add(MakeShared<FJsonValueString>(Tag.ToString()));
	}
	ActorJson->SetArrayField(TEXT("tags"), TagsArray);

	// Components if requested
	if (Options.bIncludeComponents)
	{
		ExtractComponents(Actor, ActorJson);
	}

	// Properties if requested
	if (Options.bIncludeProperties)
	{
		ExtractActorProperties(Actor, ActorJson, Options);
	}

	return ActorJson;
}

void FLevelQueryHandler::ExtractComponents(AActor* Actor, TSharedPtr<FJsonObject>& JsonObject)
{
	TArray<TSharedPtr<FJsonValue>> ComponentsArray;

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			TSharedPtr<FJsonObject> ComponentJson = MakeShared<FJsonObject>();
			ComponentJson->SetStringField(TEXT("name"), Component->GetName());
			ComponentJson->SetStringField(TEXT("class"), Component->GetClass()->GetName());
			ComponentsArray.Add(MakeShared<FJsonValueObject>(ComponentJson));
		}
	}

	JsonObject->SetArrayField(TEXT("components"), ComponentsArray);
}

void FLevelQueryHandler::ExtractActorProperties(AActor* Actor, TSharedPtr<FJsonObject>& JsonObject, const FQueryOptions& Options)
{
	TArray<TSharedPtr<FJsonValue>> PropertiesArray;

	UClass* ActorClass = Actor->GetClass();
	for (TFieldIterator<FProperty> PropIt(ActorClass); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		
		// Only include properties that are editable or visible
		if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
		{
			continue;
		}

		TSharedPtr<FJsonObject> PropertyJson = MakeShared<FJsonObject>();
		PropertyJson->SetStringField(TEXT("name"), Property->GetName());
		PropertyJson->SetStringField(TEXT("type"), Property->GetCPPType());

		// Extract value if requested
		if (Options.bIncludeDefaultValues)
		{
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Actor);
			FString Value = GetPropertyValueAsString(Property, ValuePtr);
			PropertyJson->SetStringField(TEXT("value"), Value);
		}

		PropertiesArray.Add(MakeShared<FJsonValueObject>(PropertyJson));
	}

	JsonObject->SetArrayField(TEXT("properties"), PropertiesArray);
}

FString FLevelQueryHandler::GetPropertyValueAsString(FProperty* Property, const void* ValuePtr)
{
	if (!Property || !ValuePtr)
	{
		return TEXT("null");
	}

	FString ValueString;
	Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);
	return ValueString;
}

