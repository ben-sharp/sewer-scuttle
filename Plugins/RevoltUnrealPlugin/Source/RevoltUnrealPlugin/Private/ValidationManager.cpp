// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValidationManager.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
#include "Misc/PackageName.h"

bool FValidationManager::ValidatePropertyValue(const FProperty* Property, const FString& ValueString, FString& OutError)
{
	if (!Property)
	{
		OutError = TEXT("Property is null");
		return false;
	}

	// Check if property is read-only
	if (IsPropertyReadOnly(Property))
	{
		OutError = FString::Printf(TEXT("Property '%s' is read-only"), *Property->GetName());
		return false;
	}

	// Validate numeric properties with constraints
	if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(Property))
	{
		double Value;
		if (!TryParseNumeric(ValueString, Value))
		{
			OutError = FString::Printf(TEXT("Value '%s' is not a valid number"), *ValueString);
			return false;
		}

		if (!ValidateNumericConstraints(Property, Value, OutError))
		{
			return false;
		}
	}

	// Validate bool properties
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
	{
		FString LowerValue = ValueString.ToLower();
		if (LowerValue != TEXT("true") && LowerValue != TEXT("false") && 
			LowerValue != TEXT("1") && LowerValue != TEXT("0"))
		{
			OutError = FString::Printf(TEXT("Value '%s' is not a valid boolean"), *ValueString);
			return false;
		}
	}

	return true;
}

bool FValidationManager::ValidateBlueprintEditable(const UBlueprint* Blueprint, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Blueprint is null");
		return false;
	}

	if (!Blueprint->GeneratedClass)
	{
		OutError = TEXT("Blueprint has no generated class");
		return false;
	}

	// Check if blueprint is part of engine content (shouldn't edit)
	FString PackageName = Blueprint->GetOutermost()->GetName();
	if (PackageName.StartsWith(TEXT("/Engine/")) || PackageName.StartsWith(TEXT("/Script/")))
	{
		OutError = TEXT("Cannot edit engine blueprints");
		return false;
	}

	return true;
}

bool FValidationManager::ValidateBlueprintName(const FString& Name, FString& OutError)
{
	if (Name.IsEmpty())
	{
		OutError = TEXT("Blueprint name cannot be empty");
		return false;
	}

	// Check for invalid characters
	if (Name.Contains(TEXT(" ")) || Name.Contains(TEXT(".")))
	{
		OutError = TEXT("Blueprint name cannot contain spaces or dots");
		return false;
	}

	// Should start with BP_
	if (!Name.StartsWith(TEXT("BP_")))
	{
		OutError = TEXT("Blueprint name should start with 'BP_' prefix");
		return false;
	}

	return true;
}

bool FValidationManager::ValidateAssetPath(const FString& Path, FString& OutError)
{
	if (Path.IsEmpty())
	{
		OutError = TEXT("Asset path cannot be empty");
		return false;
	}

	// Check if path is valid format
	if (!FPackageName::IsValidLongPackageName(Path, false))
	{
		OutError = FString::Printf(TEXT("Invalid asset path format: '%s'"), *Path);
		return false;
	}

	// Prevent writing to Engine content (but allow Game and Plugins)
	if (Path.StartsWith(TEXT("/Engine/")) || Path.StartsWith(TEXT("/Script/")))
	{
		OutError = TEXT("Cannot create assets in Engine or Script folders");
		return false;
	}

	return true;
}

bool FValidationManager::ValidateBlueprintCompilation(const UBlueprint* Blueprint, FString& OutError)
{
	if (!Blueprint)
	{
		OutError = TEXT("Blueprint is null");
		return false;
	}

	if (Blueprint->Status == BS_Error)
	{
		OutError = FString::Printf(TEXT("Blueprint '%s' has compilation errors"), *Blueprint->GetName());
		return false;
	}

	return true;
}

bool FValidationManager::ValidateNumericConstraints(const FProperty* Property, double Value, FString& OutError)
{
	if (!Property)
	{
		return false;
	}

	// Check ClampMin metadata
	if (Property->HasMetaData(TEXT("ClampMin")))
	{
		FString MinString = Property->GetMetaData(TEXT("ClampMin"));
		double MinValue;
		if (TryParseNumeric(MinString, MinValue) && Value < MinValue)
		{
			OutError = FString::Printf(TEXT("Value %.2f is below minimum %.2f"), Value, MinValue);
			return false;
		}
	}

	// Check ClampMax metadata
	if (Property->HasMetaData(TEXT("ClampMax")))
	{
		FString MaxString = Property->GetMetaData(TEXT("ClampMax"));
		double MaxValue;
		if (TryParseNumeric(MaxString, MaxValue) && Value > MaxValue)
		{
			OutError = FString::Printf(TEXT("Value %.2f is above maximum %.2f"), Value, MaxValue);
			return false;
		}
	}

	return true;
}

bool FValidationManager::IsPropertyReadOnly(const FProperty* Property)
{
	if (!Property)
	{
		return true;
	}

	// Check for read-only flags
	if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
	{
		return true;
	}

	// Check metadata
	if (Property->HasMetaData(TEXT("ReadOnly")))
	{
		return true;
	}

	return false;
}

bool FValidationManager::TryParseNumeric(const FString& ValueString, double& OutValue)
{
	if (ValueString.IsNumeric())
	{
		OutValue = FCString::Atod(*ValueString);
		return true;
	}

	return false;
}

