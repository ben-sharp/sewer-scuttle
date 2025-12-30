// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * Handles level creation and duplication
 */
class FLevelFactoryHandler
{
public:

	/**
	 * Create a new empty level at specified path
	 * @param LevelName Name for the new level
	 * @param PackagePath Path where level will be created
	 * @return Created level or nullptr if failed
	 */
	static UWorld* CreateLevel(const FString& LevelName, const FString& PackagePath);

	/**
	 * Duplicate an existing level to new location with new name
	 * @param SourceLevel Level to duplicate
	 * @param NewName Name for the duplicated level
	 * @param NewPath Path for the duplicated level (optional, uses source path if empty)
	 * @return Duplicated level or nullptr if failed
	 */
	static UWorld* DuplicateLevel(UWorld* SourceLevel, const FString& NewName, const FString& NewPath = FString());

	/**
	 * Find existing level by name
	 * @param LevelName Name of the level
	 * @return Found level or nullptr
	 */
	static UWorld* FindLevel(const FString& LevelName);

	/**
	 * Validate level name (must not already exist, path must be valid)
	 * @param LevelName Level name to validate
	 * @param OutError Error message if validation fails
	 * @return True if valid
	 */
	static bool ValidateLevelName(const FString& LevelName, FString& OutError);

private:

	/**
	 * Generate full package path for level
	 * @param PackagePath Base path
	 * @param LevelName Level name
	 * @return Full package path
	 */
	static FString GenerateLevelPackagePath(const FString& PackagePath, const FString& LevelName);

	/**
	 * Ensure package path is valid and formatted correctly
	 * @param PackagePath Path to validate
	 * @return Cleaned and validated path
	 */
	static FString ValidateAndCleanPath(const FString& PackagePath);
};

