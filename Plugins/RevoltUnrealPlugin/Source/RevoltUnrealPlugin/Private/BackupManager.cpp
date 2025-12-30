// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackupManager.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"

FString FBackupManager::CreateBackup(UPackage* Package)
{
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Cannot backup null package"));
		return FString();
	}

	if (!EnsureBackupDirectoryExists())
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create backup directory"));
		return FString();
	}

	// Generate backup filename
	FString PackageName = Package->GetName();
	FString AssetName = FPackageName::GetShortName(PackageName);
	FString BackupFilename = GenerateBackupFilename(AssetName);
	FString BackupPath = GetBackupDirectory() / BackupFilename;

	// Get the original package file path
	FString OriginalFilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	// Copy the package file to backup location
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CopyFile(*BackupPath, *OriginalFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Created backup at '%s'"), *BackupPath);
		return BackupPath;
	}

	UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create backup from '%s' to '%s'"), *OriginalFilePath, *BackupPath);
	return FString();
}

FString FBackupManager::CreateBlueprintBackup(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return FString();
	}

	return CreateBackup(Blueprint->GetOutermost());
}

FString FBackupManager::CreateLevelBackup(UWorld* Level)
{
	if (!Level)
	{
		return FString();
	}

	return CreateBackup(Level->GetOutermost());
}

FString FBackupManager::CreateDataAssetBackup(UDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		return FString();
	}

	return CreateBackup(DataAsset->GetOutermost());
}

bool FBackupManager::RestoreBackup(const FString& BackupPath)
{
	if (BackupPath.IsEmpty() || !FPaths::FileExists(BackupPath))
	{
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Backup file not found: '%s'"), *BackupPath);
		return false;
	}

	// Extract original asset name from backup filename
	FString BackupFilename = FPaths::GetCleanFilename(BackupPath);
	// Backup format: AssetName_backup_YYYYMMDD_HHMMSS.uasset
	// Need to extract the original name
	
	// For now, log that restore would happen
	UE_LOG(LogTemp, Warning, TEXT("RevoltPlugin: Restore from backup '%s' - Feature requires manual implementation"), *BackupPath);
	
	// TODO: Implement actual restore logic
	// This requires:
	// 1. Parse backup filename to get original asset path
	// 2. Load the backup package
	// 3. Replace the current package with backup
	// 4. Reload asset in editor

	return false;
}

TArray<FString> FBackupManager::ListBackups(const FString& AssetPath)
{
	TArray<FString> Backups;

	FString BackupDir = GetBackupDirectory();
	FString AssetName = FPackageName::GetShortName(AssetPath);

	// Find all backup files for this asset
	IFileManager& FileManager = IFileManager::Get();
	FString SearchPattern = BackupDir / (AssetName + TEXT("_backup_*"));
	
	FileManager.FindFiles(Backups, *SearchPattern, true, false);

	return Backups;
}

int32 FBackupManager::CleanOldBackups(int32 DaysToKeep)
{
	int32 DeletedCount = 0;
	FString BackupDir = GetBackupDirectory();

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> BackupFiles;
	FileManager.FindFilesRecursive(BackupFiles, *BackupDir, TEXT("*.uasset"), true, false);

	FDateTime Cutoff = FDateTime::Now() - FTimespan::FromDays(DaysToKeep);

	for (const FString& BackupFile : BackupFiles)
	{
		FDateTime FileTime = FileManager.GetTimeStamp(*BackupFile);
		if (FileTime < Cutoff)
		{
			if (FileManager.Delete(*BackupFile))
			{
				DeletedCount++;
				UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Deleted old backup '%s'"), *BackupFile);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Cleaned %d old backups"), DeletedCount);
	return DeletedCount;
}

bool FBackupManager::VerifyBackup(const FString& BackupPath)
{
	if (BackupPath.IsEmpty())
	{
		return false;
	}

	// Check if file exists
	if (!FPaths::FileExists(BackupPath))
	{
		return false;
	}

	// Check if file has content
	int64 FileSize = IFileManager::Get().FileSize(*BackupPath);
	if (FileSize <= 0)
	{
		return false;
	}

	return true;
}

FString FBackupManager::GetBackupDirectory()
{
	// Store backups in project's Saved/Backups directory
	FString ProjectDir = FPaths::ProjectSavedDir();
	return ProjectDir / TEXT("RevoltBackups");
}

FString FBackupManager::GenerateBackupFilename(const FString& OriginalAssetName)
{
	// Format: AssetName_backup_YYYYMMDD_HHMMSS.uasset
	FDateTime Now = FDateTime::Now();
	FString Timestamp = FString::Printf(TEXT("%04d%02d%02d_%02d%02d%02d"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());

	return FString::Printf(TEXT("%s_backup_%s.uasset"), *OriginalAssetName, *Timestamp);
}

bool FBackupManager::EnsureBackupDirectoryExists()
{
	FString BackupDir = GetBackupDirectory();
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*BackupDir))
	{
		if (PlatformFile.CreateDirectory(*BackupDir))
		{
			UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Created backup directory at '%s'"), *BackupDir);
			return true;
		}
		
		UE_LOG(LogTemp, Error, TEXT("RevoltPlugin: Failed to create backup directory at '%s'"), *BackupDir);
		return false;
	}

	return true;
}

