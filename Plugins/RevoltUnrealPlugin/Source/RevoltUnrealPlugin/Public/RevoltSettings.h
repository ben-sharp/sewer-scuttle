// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QueryOptions.h"
#include "RevoltSettings.generated.h"

/**
 * Configuration settings for Revolt Unreal Plugin
 * Accessible via Project Settings -> Plugins -> Revolt Plugin
 */
UCLASS(config=Editor, defaultconfig, meta=(DisplayName="Revolt Plugin Settings"))
class REVOLTUNREALPLUGIN_API URevoltSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	URevoltSettings();

	/** Auto-start HTTP server when editor loads */
	UPROPERTY(config, EditAnywhere, Category="Server")
	bool bAutoStartServer = true;

	/** Port for HTTP server */
	UPROPERTY(config, EditAnywhere, Category="Server", meta=(ClampMin=1024, ClampMax=65535))
	int32 ServerPort = 8080;

	/** Enable verbose logging */
	UPROPERTY(config, EditAnywhere, Category="Server")
	bool bEnableLogging = true;

	/** Default query depth level */
	UPROPERTY(config, EditAnywhere, Category="Query")
	EQueryDepth DefaultQueryDepth = EQueryDepth::Standard;

	/** Maximum number of actors to return per level query */
	UPROPERTY(config, EditAnywhere, Category="Query", meta=(ClampMin=1, ClampMax=10000))
	int32 MaxActorsPerQuery = 1000;

	/** Enable automatic backups before edits */
	UPROPERTY(config, EditAnywhere, Category="Safety")
	bool bAutoBackup = true;

	/** Number of days to keep backup files */
	UPROPERTY(config, EditAnywhere, Category="Safety", meta=(ClampMin=1, ClampMax=90))
	int32 BackupRetentionDays = 7;

	/** Require confirmation for all edit operations */
	UPROPERTY(config, EditAnywhere, Category="Safety")
	bool bRequireConfirmation = true;

	/** Automatically compile blueprints after editing */
	UPROPERTY(config, EditAnywhere, Category="Edit")
	bool bAutoCompile = true;

	/** Validate property values before applying */
	UPROPERTY(config, EditAnywhere, Category="Edit")
	bool bValidateBeforeEdit = true;

	/** Maximum number of blueprints to edit in a single bulk operation */
	UPROPERTY(config, EditAnywhere, Category="Edit", meta=(ClampMin=1, ClampMax=1000))
	int32 MaxBulkEditCount = 100;

	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	//~ End UDeveloperSettings Interface
};

