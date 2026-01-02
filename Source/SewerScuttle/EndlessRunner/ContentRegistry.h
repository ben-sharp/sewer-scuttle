// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ContentRegistry.generated.h"

class UTrackPieceDefinition;
class UObstacleDefinition;
class UPowerUpDefinition;
class UCollectibleDefinition;

/**
 * Registry for gathering all game content definitions for export
 */
UCLASS()
class SEWERSCUTTLE_API UContentRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** Gather all content definitions from the project */
	void GatherContent();

	/** Get gathered track pieces */
	const TArray<UTrackPieceDefinition*>& GetTrackPieces() const { return TrackPieces; }

	/** Get gathered obstacles */
	const TArray<UObstacleDefinition*>& GetObstacles() const { return Obstacles; }

	/** Get gathered power-ups */
	const TArray<UPowerUpDefinition*>& GetPowerUps() const { return PowerUps; }

	/** Get gathered collectibles */
	const TArray<UCollectibleDefinition*>& GetCollectibles() const { return Collectibles; }

	/** Find a track piece definition by its ID (AssetName) */
	UFUNCTION(BlueprintPure, Category = "Content")
	UTrackPieceDefinition* FindTrackPieceById(const FString& ContentId) const;

	/** Find an obstacle definition by its ID (AssetName) */
	UFUNCTION(BlueprintPure, Category = "Content")
	UObstacleDefinition* FindObstacleById(const FString& ContentId) const;

	/** Find a power-up definition by its ID (AssetName) */
	UFUNCTION(BlueprintPure, Category = "Content")
	UPowerUpDefinition* FindPowerUpById(const FString& ContentId) const;

	/** Find a collectible definition by its ID (AssetName) */
	UFUNCTION(BlueprintPure, Category = "Content")
	UCollectibleDefinition* FindCollectibleById(const FString& ContentId) const;

private:
	UPROPERTY()
	TArray<UTrackPieceDefinition*> TrackPieces;

	UPROPERTY()
	TArray<UObstacleDefinition*> Obstacles;

	UPROPERTY()
	TArray<UPowerUpDefinition*> PowerUps;

	UPROPERTY()
	TArray<UCollectibleDefinition*> Collectibles;
};

