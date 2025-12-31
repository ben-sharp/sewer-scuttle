// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ContentRegistry.generated.h"

class UTrackPieceDefinition;
class UObstacleDefinition;
class UPowerUpDefinition;
class UCollectibleDefinition;
class UDataAsset;

USTRUCT(BlueprintType)
struct SEWERSCUTTLE_API FContentDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Type; // track_piece, obstacle, powerup, collectible

	UPROPERTY(BlueprintReadWrite)
	FString Id; // Unique ID from data asset

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> Properties; // Type-specific properties as key-value pairs
};

/**
 * Content Registry - Collects all content definitions from data assets
 */
UCLASS()
class SEWERSCUTTLE_API UContentRegistry : public UObject
{
	GENERATED_BODY()

public:
	UContentRegistry();

	/** Collect all content definitions */
	UFUNCTION(BlueprintCallable, Category = "Content")
	void CollectAllContent();

	/** Get content by type */
	UFUNCTION(BlueprintCallable, Category = "Content")
	TArray<FContentDefinition> GetContentByType(const FString& Type) const;

	/** Get all content */
	UFUNCTION(BlueprintCallable, Category = "Content")
	TArray<FContentDefinition> GetAllContent() const { return AllContent; }

	/** Get content version */
	UFUNCTION(BlueprintPure, Category = "Content")
	FString GetContentVersion() const { return ContentVersion; }

	/** Set content version */
	UFUNCTION(BlueprintCallable, Category = "Content")
	void SetContentVersion(const FString& Version) { ContentVersion = Version; }

private:
	/** Collect track piece definitions */
	void CollectTrackPieces();

	/** Collect obstacle definitions */
	void CollectObstacles();

	/** Collect powerup definitions */
	void CollectPowerUps();

	/** Collect collectible definitions */
	void CollectCollectibles();

	/** Convert track piece definition to content definition */
	FContentDefinition ConvertTrackPiece(UTrackPieceDefinition* Definition) const;

	/** Convert obstacle definition to content definition */
	FContentDefinition ConvertObstacle(UObstacleDefinition* Definition) const;

	/** Convert powerup definition to content definition */
	FContentDefinition ConvertPowerUp(UPowerUpDefinition* Definition) const;

	/** Convert collectible definition to content definition */
	FContentDefinition ConvertCollectible(UCollectibleDefinition* Definition) const;

	UPROPERTY()
	TArray<FContentDefinition> AllContent;

	UPROPERTY()
	FString ContentVersion = TEXT("1.0.0");
};

