// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UBlueprint;
class UClass;

/**
 * Handles blueprint creation and duplication
 */
class FBlueprintFactoryHandler
{
public:

	/**
	 * Create a new blueprint from a parent class
	 * @param ParentClass Parent class for the blueprint
	 * @param BlueprintName Name for the new blueprint
	 * @param PackagePath Path where blueprint will be created
	 * @return Created blueprint or nullptr if failed
	 */
	static UBlueprint* CreateBlueprint(UClass* ParentClass, const FString& BlueprintName, const FString& PackagePath);

	/**
	 * Duplicate an existing blueprint
	 * @param SourceBlueprint Blueprint to duplicate
	 * @param NewName Name for the duplicated blueprint
	 * @param NewPath Path for the duplicated blueprint (optional, uses source path if empty)
	 * @return Duplicated blueprint or nullptr if failed
	 */
	static UBlueprint* DuplicateBlueprint(UBlueprint* SourceBlueprint, const FString& NewName, const FString& NewPath = FString());

	/**
	 * Find a parent class by name
	 * @param ClassName Name of the class
	 * @return Found class or nullptr
	 */
	static UClass* FindClass(const FString& ClassName);

private:

	/**
	 * Generate full package path for blueprint
	 * @param PackagePath Base path
	 * @param BlueprintName Blueprint name
	 * @return Full package path
	 */
	static FString GeneratePackagePath(const FString& PackagePath, const FString& BlueprintName);

	/**
	 * Ensure package path is valid and formatted correctly
	 * @param PackagePath Path to validate
	 * @return Cleaned and validated path
	 */
	static FString ValidateAndCleanPath(const FString& PackagePath);
};

