// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TrackPiece.h"
#include "TrackPieceDefinition.generated.h"

class UStaticMesh;
class ATrackPiece;

USTRUCT(BlueprintType)
struct FTrackPieceMeshSet
{
	GENERATED_BODY()

	/** Floor mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	UStaticMesh* FloorMesh = nullptr;

	/** Left wall mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	UStaticMesh* LeftWallMesh = nullptr;

	/** Flip left wall horizontally (useful when reusing same mesh for both sides) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	bool bFlipLeftWall = false;

	/** Right wall mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	UStaticMesh* RightWallMesh = nullptr;

	/** Flip right wall horizontally (useful when reusing same mesh for both sides) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	bool bFlipRightWall = false;

	/** Ceiling mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
	UStaticMesh* CeilingMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FTrackPieceSpawnConfig
{
	GENERATED_BODY()

	/** Lane (0 = Left, 1 = Center, 2 = Right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Lane = 1;

	/** Forward position along track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardPosition = 0.0f;

	/** Spawn type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnPointType SpawnType = ESpawnPointType::Coin;

	/** Class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpawnClass;

	/** Spawn probability (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 1.0f;

	/** Optional: Name of a Scene component in the track piece blueprint to use as spawn position (overrides Lane/ForwardPosition) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Spawn Position Component Name"))
	FString SpawnPositionComponentName;
};

/**
 * Data asset defining a track piece configuration
 * Used to generate track pieces with specific meshes and spawn points
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UTrackPieceDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of this track piece definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FString PieceName;

	/** Length of track piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float Length = 1000.0f;

	/** Use a blueprint actor instead of assembling meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (DisplayName = "Use Blueprint Actor"))
	bool bUseBlueprintActor = false;

	/** Blueprint class to spawn (if bUseBlueprintActor is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (EditCondition = "bUseBlueprintActor", DisplayName = "Blueprint Actor Class"))
	TSubclassOf<ATrackPiece> BlueprintActorClass;

	/** Name of the Scene Component in the blueprint to use as Start Connection (leave empty to use default "StartConnection") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (EditCondition = "bUseBlueprintActor", DisplayName = "Start Connection Component Name"))
	FString StartConnectionComponentName;

	/** Names of Scene Components in the blueprint to use as End Connections (for forks/turns - leave empty to use default "EndConnection") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (EditCondition = "bUseBlueprintActor", DisplayName = "End Connection Component Names"))
	TArray<FString> EndConnectionComponentNames;

	/** Meshes to use for this track piece (only used if bUseBlueprintActor is false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes", meta = (EditCondition = "!bUseBlueprintActor"))
	FTrackPieceMeshSet Meshes;

	/** Spawn configurations for this track piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<FTrackPieceSpawnConfig> SpawnConfigs;

	/** Minimum difficulty level to use this piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0"))
	int32 MinDifficulty = 0;

	/** Maximum difficulty level to use this piece (-1 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	int32 MaxDifficulty = -1;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ClampMin = "1"))
	int32 SelectionWeight = 1;

	/** Mark this piece as the starting/first piece (only one should be marked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (DisplayName = "Is First Piece"))
	bool bIsFirstPiece = false;
};

