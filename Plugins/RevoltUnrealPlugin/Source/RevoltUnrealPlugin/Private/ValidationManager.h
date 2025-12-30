// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class FProperty;

/**
 * Handles validation for edit operations
 * Prevents invalid modifications and ensures data integrity
 */
class FValidationManager
{
public:

	/**
	 * Validate if a property can be edited
	 * @param Property Property to validate
	 * @param ValueString New value as string
	 * @param OutError Error message if validation fails
	 * @return True if valid
	 */
	static bool ValidatePropertyValue(const FProperty* Property, const FString& ValueString, FString& OutError);

	/**
	 * Validate if a blueprint can be edited
	 * @param Blueprint Blueprint to validate
	 * @param OutError Error message if validation fails
	 * @return True if valid
	 */
	static bool ValidateBlueprintEditable(const UBlueprint* Blueprint, FString& OutError);

	/**
	 * Validate blueprint name for creation
	 * @param Name Proposed blueprint name
	 * @param OutError Error message if validation fails
	 * @return True if valid
	 */
	static bool ValidateBlueprintName(const FString& Name, FString& OutError);

	/**
	 * Validate asset path for creation
	 * @param Path Proposed asset path
	 * @param OutError Error message if validation fails
	 * @return True if valid
	 */
	static bool ValidateAssetPath(const FString& Path, FString& OutError);

	/**
	 * Validate if a blueprint compiled successfully
	 * @param Blueprint Blueprint to check
	 * @param OutError Error message if validation fails
	 * @return True if compiled without errors
	 */
	static bool ValidateBlueprintCompilation(const UBlueprint* Blueprint, FString& OutError);

	/**
	 * Validate numeric value is within property metadata constraints
	 * @param Property Numeric property
	 * @param Value Value to validate
	 * @param OutError Error message if validation fails
	 * @return True if within constraints
	 */
	static bool ValidateNumericConstraints(const FProperty* Property, double Value, FString& OutError);

private:

	/**
	 * Check if property is read-only
	 * @param Property Property to check
	 * @return True if read-only
	 */
	static bool IsPropertyReadOnly(const FProperty* Property);

	/**
	 * Parse string to numeric value
	 * @param ValueString String to parse
	 * @param OutValue Parsed value
	 * @return True if parsing successful
	 */
	static bool TryParseNumeric(const FString& ValueString, double& OutValue);
};

