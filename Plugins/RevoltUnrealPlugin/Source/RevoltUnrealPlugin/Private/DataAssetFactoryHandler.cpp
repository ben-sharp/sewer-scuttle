// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataAssetFactoryHandler.h"
#include "Factories/DataAssetFactory.h"
#include "Engine/DataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectIterator.h"
#include "BlueprintEditHandler.h"
#include "ValidationManager.h"
#include "BackupManager.h"
#include "TransactionManager.h"
#include "QueryOptions.h"
#include "Misc/FileHelper.h"

UDataAsset* FDataAssetFactoryHandler::CreateDataAsset(UClass* DataAssetClass, const FString& AssetName, const FString& PackagePath)
{
	if (!DataAssetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: DataAssetClass is null"));
		return nullptr;
	}

	if (!DataAssetClass->IsChildOf(UDataAsset::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Class '%s' is not a child of UDataAsset"), *DataAssetClass->GetName());
		return nullptr;
	}

	if (AssetName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Asset name is empty"));
		return nullptr;
	}

	// Generate full package path
	FString FullPackagePath = GeneratePackagePath(PackagePath, AssetName);

	// Create package
	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create package '%s'"), *FullPackagePath);
		return nullptr;
	}

	// Create Data Asset using factory
	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
	Factory->DataAssetClass = DataAssetClass;

	UObject* NewAsset = Factory->FactoryCreateNew(
		DataAssetClass,
		Package,
		*AssetName,
		RF_Public | RF_Standalone,
		nullptr,
		GWarn
	);

	UDataAsset* NewDataAsset = Cast<UDataAsset>(NewAsset);
	if (!NewDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create data asset '%s'"), *AssetName);
		return nullptr;
	}

	// Mark package dirty and save
	Package->MarkPackageDirty();
	
	if (SaveDataAsset(NewDataAsset))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully created Data Asset '%s' at '%s'"), *AssetName, *FullPackagePath);
		
		// Notify asset registry
		FAssetRegistryModule::AssetCreated(NewDataAsset);
		
		return NewDataAsset;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save new Data Asset '%s'"), *AssetName);
	return nullptr;
}

UClass* FDataAssetFactoryHandler::FindDataAssetClass(const FString& ClassName)
{
	// 1. Try as exact path (FindObject)
	UClass* FoundClass = FindObject<UClass>(nullptr, *ClassName);
	if (FoundClass)
	{
		return FoundClass;
	}

	// 2. Try as exact path (LoadObject)
	FoundClass = LoadObject<UClass>(nullptr, *ClassName);
	if (FoundClass)
	{
		return FoundClass;
	}

	// 3. If it's a short name, try to find it
	if (!ClassName.Contains(TEXT("/")))
	{
		// Try common paths first
		TArray<FString> Prefixes;
		Prefixes.Add(TEXT("/Script/Engine."));
		Prefixes.Add(TEXT("/Script/CoreUObject."));
		
		for (const FString& Prefix : Prefixes)
		{
			FString FullPath = Prefix + ClassName;
			FoundClass = FindObject<UClass>(nullptr, *FullPath);
			if (FoundClass) return FoundClass;
			
			FoundClass = LoadObject<UClass>(nullptr, *FullPath);
			if (FoundClass) return FoundClass;
		}

		// Iterate all classes in memory
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->GetName() == ClassName)
			{
				return *It;
			}
		}
	}
	
	return nullptr;
}

bool FDataAssetFactoryHandler::SaveDataAsset(UDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		return false;
	}

	UPackage* Package = DataAsset->GetOutermost();
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Could not get package for data asset '%s'"), *DataAsset->GetName());
		return false;
	}

	// Save the package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	bool bSaved = UPackage::SavePackage(Package, DataAsset, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Saved data asset '%s' to '%s'"), *DataAsset->GetName(), *PackageFileName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save data asset '%s'"), *DataAsset->GetName());
	}

	return bSaved;
}

FString FDataAssetFactoryHandler::GeneratePackagePath(const FString& PackagePath, const FString& AssetName)
{
	FString CleanPath = ValidateAndCleanPath(PackagePath);
	
	// Ensure path ends without trailing slash
	if (CleanPath.EndsWith(TEXT("/")))
	{
		CleanPath = CleanPath.LeftChop(1);
	}

	// Return full path
	return CleanPath / AssetName;
}

FString FDataAssetFactoryHandler::ValidateAndCleanPath(const FString& PackagePath)
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

UDataAsset* FDataAssetFactoryHandler::FindDataAsset(const FString& DataAssetName)
{
	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for data asset by name
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (AssetData.AssetName.ToString().Equals(DataAssetName, ESearchCase::IgnoreCase) ||
			AssetData.AssetName.ToString().Contains(DataAssetName, ESearchCase::IgnoreCase))
		{
			UDataAsset* DataAsset = Cast<UDataAsset>(AssetData.GetAsset());
			if (DataAsset)
			{
				return DataAsset;
			}
		}
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FDataAssetFactoryHandler::GetAllDataAssets(UClass* DataAssetClass, const FQueryOptions& Options)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all data assets
	TArray<FAssetData> AssetDataList;
	if (DataAssetClass)
	{
		AssetRegistry.GetAssetsByClass(DataAssetClass->GetClassPathName(), AssetDataList, true);
	}
	else
	{
		AssetRegistry.GetAssetsByClass(UDataAsset::StaticClass()->GetClassPathName(), AssetDataList, true);
	}

	TArray<TSharedPtr<FJsonValue>> DataAssetArray;

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDataAsset* DataAsset = Cast<UDataAsset>(AssetData.GetAsset());
		if (DataAsset)
		{
			TSharedPtr<FJsonObject> AssetInfo = MakeShared<FJsonObject>();
			AssetInfo->SetStringField(TEXT("name"), DataAsset->GetName());
			AssetInfo->SetStringField(TEXT("path"), DataAsset->GetPathName());
			AssetInfo->SetStringField(TEXT("class"), DataAsset->GetClass()->GetPathName());

			// Add basic properties if requested
			if (Options.bIncludeProperties || Options.DepthLevel >= EQueryDepth::Standard)
			{
				TArray<TSharedPtr<FJsonValue>> PropertiesArray;
				ExtractDataAssetProperties(DataAsset->GetClass(), PropertiesArray, Options);
				AssetInfo->SetArrayField(TEXT("properties"), PropertiesArray);
			}

			// Add default values if requested
			if (Options.bIncludeDefaultValues || Options.DepthLevel >= EQueryDepth::Deep)
			{
				TArray<TSharedPtr<FJsonValue>> DefaultsArray;
				ExtractDataAssetDefaults(DataAsset->GetClass(), DefaultsArray, Options);
				AssetInfo->SetArrayField(TEXT("defaults"), DefaultsArray);
			}

			DataAssetArray.Add(MakeShared<FJsonValueObject>(AssetInfo));
		}
	}

	Result->SetArrayField(TEXT("data_assets"), DataAssetArray);
	Result->SetNumberField(TEXT("count"), DataAssetArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	return Result;
}

TSharedPtr<FJsonObject> FDataAssetFactoryHandler::GetDataAssetInfo(const FString& DataAssetName, const FQueryOptions& Options)
{
	UDataAsset* DataAsset = FindDataAsset(DataAssetName);
	if (!DataAsset)
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), DataAsset->GetName());
	Result->SetStringField(TEXT("path"), DataAsset->GetPathName());
	Result->SetStringField(TEXT("class"), DataAsset->GetClass()->GetPathName());
	Result->SetBoolField(TEXT("success"), true);

	// Add properties if requested
	if (Options.bIncludeProperties || Options.DepthLevel >= EQueryDepth::Standard)
	{
		TArray<TSharedPtr<FJsonValue>> PropertiesArray;
		ExtractDataAssetProperties(DataAsset->GetClass(), PropertiesArray, Options);
		Result->SetArrayField(TEXT("properties"), PropertiesArray);
	}

	// Add default/current values if requested
	if (Options.bIncludeDefaultValues || Options.DepthLevel >= EQueryDepth::Deep)
	{
		TArray<TSharedPtr<FJsonValue>> DefaultsArray;
		ExtractDataAssetDefaults(DataAsset->GetClass(), DefaultsArray, Options);

		// Also extract current values
		TArray<TSharedPtr<FJsonValue>> CurrentValuesArray;
		ExtractDataAssetCurrentValues(DataAsset, CurrentValuesArray, Options);

		Result->SetArrayField(TEXT("defaults"), DefaultsArray);
		Result->SetArrayField(TEXT("current_values"), CurrentValuesArray);
	}

	// Add full details if requested
	if (Options.DepthLevel >= EQueryDepth::Full)
	{
		// For full depth, include everything plus functions if they exist
		TArray<TSharedPtr<FJsonValue>> FunctionsArray;
		ExtractDataAssetFunctions(DataAsset->GetClass(), FunctionsArray, Options);
		Result->SetArrayField(TEXT("functions"), FunctionsArray);
	}

	return Result;
}

bool FDataAssetFactoryHandler::EditDataAssetProperties(const FString& DataAssetName, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutChanges)
{
	UDataAsset* DataAsset = FindDataAsset(DataAssetName);
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Data Asset '%s' not found"), *DataAssetName);
		return false;
	}

	bool bAllSucceeded = true;

	for (const auto& PropertyPair : Properties)
	{
		FName PropertyName(*PropertyPair.Key);
		FString OldValue;

		TSharedPtr<FJsonObject> ChangeJson = MakeShared<FJsonObject>();
		ChangeJson->SetStringField(TEXT("property"), PropertyPair.Key);
		ChangeJson->SetStringField(TEXT("new_value"), PropertyPair.Value);

		// Validate the property exists and is editable
		FProperty* Property = DataAsset->GetClass()->FindPropertyByName(PropertyName);
		if (!Property)
		{
			ChangeJson->SetBoolField(TEXT("success"), false);
			ChangeJson->SetStringField(TEXT("error"), TEXT("Property not found"));
			bAllSucceeded = false;
		}
		else
		{
			// Get old value
			OldValue = FBlueprintEditHandler::GetPropertyValue(DataAsset, Property);
			ChangeJson->SetStringField(TEXT("old_value"), OldValue);

			// Validate the new value
			FString ValidationError;
			if (!FValidationManager::ValidatePropertyValue(Property, PropertyPair.Value, ValidationError))
			{
				ChangeJson->SetBoolField(TEXT("success"), false);
				ChangeJson->SetStringField(TEXT("error"), ValidationError);
				bAllSucceeded = false;
			}
			else
			{
				// Set the new value
				if (FBlueprintEditHandler::SetPropertyValue(DataAsset, Property, PropertyPair.Value))
				{
					ChangeJson->SetBoolField(TEXT("success"), true);
				}
				else
				{
					ChangeJson->SetBoolField(TEXT("success"), false);
					ChangeJson->SetStringField(TEXT("error"), TEXT("Failed to set property value"));
					bAllSucceeded = false;
				}
			}
		}

		OutChanges.Add(ChangeJson);
	}

	// Mark package dirty if any changes were made
	if (bAllSucceeded && Properties.Num() > 0)
	{
		DataAsset->MarkPackageDirty();

		// Save the data asset
		if (!SaveDataAsset(DataAsset))
		{
			UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to save Data Asset '%s' after editing"), *DataAssetName);
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Successfully edited Data Asset '%s'"), *DataAssetName);
	}

	return bAllSucceeded;
}

// ============================================================================
// Helper Functions for Data Asset Extraction
// ============================================================================

void FDataAssetFactoryHandler::ExtractDataAssetProperties(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& PropertiesArray, const FQueryOptions& Options)
{
	if (!DataAssetClass) return;

	for (TFieldIterator<FProperty> PropertyIt(DataAssetClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;

		// Skip if not a UPROPERTY or not editable
		if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
		{
			continue;
		}

		TSharedPtr<FJsonObject> PropertyJson = MakeShared<FJsonObject>();
		PropertyJson->SetStringField(TEXT("name"), Property->GetName());
		PropertyJson->SetStringField(TEXT("type"), Property->GetCPPType());

		// Add category if available
		FString Category = Property->GetMetaData(TEXT("Category"));
		if (!Category.IsEmpty())
		{
			PropertyJson->SetStringField(TEXT("category"), Category);
		}

		// Add flags
		TArray<TSharedPtr<FJsonValue>> FlagsArray;
		if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			FlagsArray.Add(MakeShared<FJsonValueString>(TEXT("BlueprintVisible")));
		}
		if (Property->HasAnyPropertyFlags(CPF_Edit))
		{
			FlagsArray.Add(MakeShared<FJsonValueString>(TEXT("Edit")));
		}
		if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
		{
			FlagsArray.Add(MakeShared<FJsonValueString>(TEXT("BlueprintReadOnly")));
		}
		PropertyJson->SetArrayField(TEXT("flags"), FlagsArray);

		PropertiesArray.Add(MakeShared<FJsonValueObject>(PropertyJson));
	}
}

void FDataAssetFactoryHandler::ExtractDataAssetDefaults(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& DefaultsArray, const FQueryOptions& Options)
{
	if (!DataAssetClass) return;

	UObject* CDO = DataAssetClass->GetDefaultObject();
	if (!CDO) return;

	for (TFieldIterator<FProperty> PropertyIt(DataAssetClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;

		// Skip if not a UPROPERTY or not editable
		if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
		{
			continue;
		}

		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
		if (ValuePtr)
		{
			FString ValueString;
			Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);

			TSharedPtr<FJsonObject> DefaultJson = MakeShared<FJsonObject>();
			DefaultJson->SetStringField(TEXT("property"), Property->GetName());
			DefaultJson->SetStringField(TEXT("default_value"), ValueString);
			DefaultsArray.Add(MakeShared<FJsonValueObject>(DefaultJson));
		}
	}
}

void FDataAssetFactoryHandler::ExtractDataAssetCurrentValues(UDataAsset* DataAsset, TArray<TSharedPtr<FJsonValue>>& CurrentValuesArray, const FQueryOptions& Options)
{
	if (!DataAsset) return;

	for (TFieldIterator<FProperty> PropertyIt(DataAsset->GetClass(), EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;

		// Skip if not a UPROPERTY or not editable
		if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
		{
			continue;
		}

		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(DataAsset);
		if (ValuePtr)
		{
			FString ValueString;
			Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);

			TSharedPtr<FJsonObject> CurrentJson = MakeShared<FJsonObject>();
			CurrentJson->SetStringField(TEXT("property"), Property->GetName());
			CurrentJson->SetStringField(TEXT("current_value"), ValueString);
			CurrentValuesArray.Add(MakeShared<FJsonValueObject>(CurrentJson));
		}
	}
}

void FDataAssetFactoryHandler::ExtractDataAssetFunctions(UClass* DataAssetClass, TArray<TSharedPtr<FJsonValue>>& FunctionsArray, const FQueryOptions& Options)
{
	if (!DataAssetClass) return;

	for (TFieldIterator<UFunction> FunctionIt(DataAssetClass, EFieldIteratorFlags::IncludeSuper); FunctionIt; ++FunctionIt)
	{
		UFunction* Function = *FunctionIt;

		// Skip if not a UFUNCTION or not callable
		if (!Function->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintEvent))
		{
			continue;
		}

		TSharedPtr<FJsonObject> FunctionJson = MakeShared<FJsonObject>();
		FunctionJson->SetStringField(TEXT("name"), Function->GetName());
		FunctionJson->SetStringField(TEXT("return_type"), Function->GetReturnProperty() ? Function->GetReturnProperty()->GetCPPType() : TEXT("void"));

		// Extract parameters
		TArray<TSharedPtr<FJsonValue>> ParametersArray;
		for (TFieldIterator<FProperty> ParamIt(Function); ParamIt; ++ParamIt)
		{
			FProperty* Param = *ParamIt;

			// Skip return property
			if (Param == Function->GetReturnProperty())
			{
				continue;
			}

			TSharedPtr<FJsonObject> ParamJson = MakeShared<FJsonObject>();
			ParamJson->SetStringField(TEXT("name"), Param->GetName());
			ParamJson->SetStringField(TEXT("type"), Param->GetCPPType());
			ParametersArray.Add(MakeShared<FJsonValueObject>(ParamJson));
		}

		FunctionJson->SetArrayField(TEXT("parameters"), ParametersArray);
		FunctionsArray.Add(MakeShared<FJsonValueObject>(FunctionJson));
	}
}
