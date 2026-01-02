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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> FloorMesh;

	/** Left wall mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> LeftWallMesh;

	/** Flip left wall horizontally (useful when reusing same mesh for both sides) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipLeftWall = false;

	/** Right wall mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> RightWallMesh;

	/** Flip right wall horizontally (useful when reusing same mesh for both sides) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipRightWall = false;

	/** Ceiling mesh (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> CeilingMesh;
};

UENUM(BlueprintType)
enum class ETrackPieceType : uint8
{
	Normal,
	Shop,
	Boss,
	Start
};

namespace TrackPieceUtils
{
	static FString ToString(ETrackPieceType Type)
	{
		switch (Type)
		{
			case ETrackPieceType::Shop: return TEXT("Shop");
			case ETrackPieceType::Boss: return TEXT("Boss");
			case ETrackPieceType::Start: return TEXT("Start");
			case ETrackPieceType::Normal:
			default: return TEXT("Normal");
		}
	}

	static ETrackPieceType FromString(const FString& TypeStr)
	{
		if (TypeStr.Equals(TEXT("Shop"), ESearchCase::IgnoreCase)) return ETrackPieceType::Shop;
		if (TypeStr.Equals(TEXT("Boss"), ESearchCase::IgnoreCase)) return ETrackPieceType::Boss;
		if (TypeStr.Equals(TEXT("Start"), ESearchCase::IgnoreCase)) return ETrackPieceType::Start;
		return ETrackPieceType::Normal;
	}
}

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

	/** Type of track piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	ETrackPieceType PieceType = ETrackPieceType::Normal;

	/** Is this the first piece of the track? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	bool bIsFirstPiece = false;

	/** Length of track piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float Length = 1000.0f;

	/** Width of each lane for this specific piece type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float LaneWidth = 200.0f;

	/** Whether to use a blueprint actor instead of procedural mesh generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	bool bUseBlueprintActor = false;

	/** The blueprint actor class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint", meta = (EditCondition = "bUseBlueprintActor"))
	TSubclassOf<ATrackPiece> BlueprintActorClass;

	/** Component name for start connection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint", meta = (EditCondition = "bUseBlueprintActor"))
	FString StartConnectionComponentName = TEXT("StartConnection");

	/** Component names for end connections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint", meta = (EditCondition = "bUseBlueprintActor"))
	TArray<FString> EndConnectionComponentNames;

	/** Meshes to use for this track piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes", meta = (EditCondition = "!bUseBlueprintActor"))
	FTrackPieceMeshSet Meshes;

	/** Minimum difficulty level to use this piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0"))
	int32 MinDifficulty = 0;

	/** Maximum difficulty level to use this piece (-1 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	int32 MaxDifficulty = -1;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ClampMin = "1"))
	int32 SelectionWeight = 1;
};

