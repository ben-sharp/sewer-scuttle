// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPackage;
class UBlueprint;
class UDataAsset;

/**
 * Manages automatic backups of assets before modification
 * Provides restore functionality for failed operations
 */
class FBackupManager
{
public:

	/**
	 * Create a backup of a package
	 * @param Package Package to backup
	 * @return Path to backup file, empty if failed
	 */
	static FString CreateBackup(UPackage* Package);

	/**
	 * Create a backup of a blueprint
	 * @param Blueprint Blueprint to backup
	 * @return Path to backup file, empty if failed
	 */
	static FString CreateBlueprintBackup(UBlueprint* Blueprint);

	/**
	 * Create a backup of a level
	 * @param Level Level/World to backup
	 * @return Path to backup file, empty if failed
	 */
	static FString CreateLevelBackup(UWorld* Level);

	/**
	 * Create a backup of a data asset
	 * @param DataAsset Data asset to backup
	 * @return Path to backup file, empty if failed
	 */
	static FString CreateDataAssetBackup(UDataAsset* DataAsset);

	/**
	 * Restore a package from backup
	 * @param BackupPath Path to backup file
	 * @return True if restore successful
	 */
	static bool RestoreBackup(const FString& BackupPath);

	/**
	 * List all backups for an asset
	 * @param AssetPath Path to asset
	 * @return Array of backup file paths
	 */
	static TArray<FString> ListBackups(const FString& AssetPath);

	/**
	 * Delete old backups
	 * @param DaysToKeep Number of days to keep backups
	 * @return Number of backups deleted
	 */
	static int32 CleanOldBackups(int32 DaysToKeep = 7);

	/**
	 * Verify a backup file is valid
	 * @param BackupPath Path to backup file
	 * @return True if backup is valid
	 */
	static bool VerifyBackup(const FString& BackupPath);

	/**
	 * Get backup directory path
	 * @return Full path to backup directory
	 */
	static FString GetBackupDirectory();

private:

	/**
	 * Generate backup filename with timestamp
	 * @param OriginalAssetName Original asset name
	 * @return Timestamped backup filename
	 */
	static FString GenerateBackupFilename(const FString& OriginalAssetName);

	/**
	 * Ensure backup directory exists
	 * @return True if directory exists or was created
	 */
	static bool EnsureBackupDirectoryExists();
};

