// Copyright Epic Games, Inc. All Rights Reserved.

#include "BulkEditHandler.h"
#include "BlueprintEditHandler.h"
#include "BackupManager.h"
#include "Engine/Blueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Dom/JsonValue.h"

TArray<UBlueprint*> FBulkEditHandler::FilterBlueprints(const FBulkFilter& Filter)
{
	TArray<UBlueprint*> MatchingBlueprints;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Get all blueprints
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		// Skip engine content
		if (AssetData.PackageName.ToString().StartsWith(TEXT("/Engine/")))
		{
			continue;
		}

		UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
		if (Blueprint && MatchesFilter(Blueprint, Filter))
		{
			MatchingBlueprints.Add(Blueprint);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Bulk filter matched %d blueprints"), MatchingBlueprints.Num());
	return MatchingBlueprints;
}

bool FBulkEditHandler::BulkEditProperties(const TArray<UBlueprint*>& Blueprints, const TMap<FString, FString>& Properties, TArray<TSharedPtr<FJsonObject>>& OutResults)
{
	bool bAllSucceeded = true;

	for (UBlueprint* Blueprint : Blueprints)
	{
		TSharedPtr<FJsonObject> ResultJson = MakeShared<FJsonObject>();
		ResultJson->SetStringField(TEXT("blueprint"), Blueprint->GetName());

		// Create backup
		FString BackupPath = FBackupManager::CreateBlueprintBackup(Blueprint);
		if (BackupPath.IsEmpty())
		{
			ResultJson->SetBoolField(TEXT("success"), false);
			ResultJson->SetStringField(TEXT("error"), TEXT("Failed to create backup"));
			OutResults.Add(ResultJson);
			bAllSucceeded = false;
			continue;
		}

		// For each property, parse expression and apply
		TMap<FString, FString> ResolvedProperties;
		for (const auto& PropertyPair : Properties)
		{
			// Get current value
			UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
			FProperty* Property = Blueprint->GeneratedClass->FindPropertyByName(*PropertyPair.Key);
			
			if (Property && CDO)
			{
				FString CurrentValue = FBlueprintEditHandler::GetPropertyValue(CDO, Property);
				FString NewValue = ParseValueExpression(PropertyPair.Value, CurrentValue);
				ResolvedProperties.Add(PropertyPair.Key, NewValue);
			}
			else
			{
				// If property doesn't exist, use value as-is
				ResolvedProperties.Add(PropertyPair.Key, PropertyPair.Value);
			}
		}

		// Edit properties
		TArray<TSharedPtr<FJsonObject>> Changes;
		bool bEditSuccess = FBlueprintEditHandler::EditBlueprintProperties(Blueprint, ResolvedProperties, Changes);

		if (bEditSuccess)
		{
			// Compile and save
			FBlueprintEditHandler::CompileBlueprint(Blueprint);
			FBlueprintEditHandler::SaveBlueprint(Blueprint);

			ResultJson->SetBoolField(TEXT("success"), true);
			ResultJson->SetArrayField(TEXT("changes"), ConvertToJsonValueArray(Changes));
		}
		else
		{
			// Restore backup
			FBackupManager::RestoreBackup(BackupPath);
			ResultJson->SetBoolField(TEXT("success"), false);
			ResultJson->SetStringField(TEXT("error"), TEXT("Failed to edit properties"));
			bAllSucceeded = false;
		}

		OutResults.Add(ResultJson);
	}

	return bAllSucceeded;
}

FString FBulkEditHandler::ParseValueExpression(const FString& Expression, const FString& CurrentValue)
{
	// Check if expression contains an operator
	if (Expression.StartsWith(TEXT("*")) || Expression.StartsWith(TEXT("+")) || 
		Expression.StartsWith(TEXT("-")) || Expression.StartsWith(TEXT("/")))
	{
		// Extract operator and operand
		FString Operator = Expression.Left(1);
		FString OperandStr = Expression.RightChop(1);

		// Try to parse as numeric
		if (CurrentValue.IsNumeric() && OperandStr.IsNumeric())
		{
			double Current = FCString::Atod(*CurrentValue);
			double Operand = FCString::Atod(*OperandStr);
			double Result = ApplyOperation(Operator, Current, Operand);
			
			return FString::SanitizeFloat(Result);
		}
	}

	// No operator or not numeric, return expression as-is
	return Expression;
}

bool FBulkEditHandler::MatchesFilter(const UBlueprint* Blueprint, const FBulkFilter& Filter)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return false;
	}

	// Check type filter (weapon, character, ai)
	if (!Filter.Type.IsEmpty())
	{
		FString BlueprintName = Blueprint->GetName().ToLower();
		FString BlueprintPath = Blueprint->GetPathName().ToLower();
		FString TypeLower = Filter.Type.ToLower();

		bool bMatchesType = BlueprintName.Contains(TypeLower) || BlueprintPath.Contains(TypeLower);
		if (!bMatchesType)
		{
			return false;
		}
	}

	// Check parent class filter
	if (!Filter.ParentClass.IsEmpty())
	{
		UClass* ParentClass = Blueprint->GeneratedClass->GetSuperClass();
		while (ParentClass)
		{
			if (ParentClass->GetName().Contains(Filter.ParentClass))
			{
				break;
			}
			ParentClass = ParentClass->GetSuperClass();
		}

		if (!ParentClass)
		{
			return false;
		}
	}

	// Check name pattern (simple wildcard)
	if (!Filter.NamePattern.IsEmpty())
	{
		FString Pattern = Filter.NamePattern;
		Pattern.ReplaceInline(TEXT("*"), TEXT(""));
		
		if (!Blueprint->GetName().Contains(Pattern))
		{
			return false;
		}
	}

	// Check path contains
	if (!Filter.PathContains.IsEmpty())
	{
		if (!Blueprint->GetPathName().Contains(Filter.PathContains))
		{
			return false;
		}
	}

	return true;
}

double FBulkEditHandler::ApplyOperation(const FString& Operator, double CurrentValue, double Operand)
{
	if (Operator == TEXT("*"))
	{
		return CurrentValue * Operand;
	}
	else if (Operator == TEXT("+"))
	{
		return CurrentValue + Operand;
	}
	else if (Operator == TEXT("-"))
	{
		return CurrentValue - Operand;
	}
	else if (Operator == TEXT("/"))
	{
		if (Operand != 0.0)
		{
			return CurrentValue / Operand;
		}
	}

	return CurrentValue;
}

TArray<TSharedPtr<FJsonValue>> FBulkEditHandler::ConvertToJsonValueArray(const TArray<TSharedPtr<FJsonObject>>& Objects)
{
	TArray<TSharedPtr<FJsonValue>> Values;
	for (const auto& Obj : Objects)
	{
		Values.Add(MakeShared<FJsonValueObject>(Obj));
	}
	return Values;
}


