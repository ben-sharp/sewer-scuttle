// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSplineManager.generated.h"

class ATrackPiece;

/**
 * Control point for a lane at track piece connection
 */
USTRUCT(BlueprintType)
struct FLaneControlPoint
{
	GENERATED_BODY()

	/** World position of the control point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Tangent direction (normalized) - direction the lane is heading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction = FVector(1.0f, 0.0f, 0.0f);

	/** Turn angle in degrees (0 = straight, positive = right turn, negative = left turn, ±90 = sharp turn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float TurnAngle = 0.0f;

	/** If true, discrete angle change (sharp corner); if false, smooth curve (spline interpolation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSharpCorner = false;

	FLaneControlPoint()
		: Position(FVector::ZeroVector)
		, Direction(FVector(1.0f, 0.0f, 0.0f))
		, TurnAngle(0.0f)
		, bSharpCorner(false)
	{}
};

/**
 * Per-track-piece lane configuration
 */
USTRUCT(BlueprintType)
struct FTrackPieceLaneConfig
{
	GENERATED_BODY()

	/** Control point where lane enters this piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLaneControlPoint StartPoint;

	/** Control point where lane exits this piece */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLaneControlPoint EndPoint;

	/** Vertical change over piece length (elevation change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationChange = 0.0f;

	FTrackPieceLaneConfig()
		: ElevationChange(0.0f)
	{}
};

/**
 * Lane spline manager - maintains continuous splines for each lane
 * Builds splines from control points contributed by track pieces
 */
UCLASS()
class SEWERSCUTTLE_API ALaneSplineManager : public AActor
{
	GENERATED_BODY()
	
public:
	ALaneSplineManager();

	virtual void BeginPlay() override;

	/** Register control points from a track piece */
	UFUNCTION(BlueprintCallable, Category = "Lane Spline")
	void RegisterTrackPiece(ATrackPiece* TrackPiece, const TArray<FTrackPieceLaneConfig>& LaneConfigs);

	/** Remove track piece from splines (when piece is destroyed) */
	UFUNCTION(BlueprintCallable, Category = "Lane Spline")
	void UnregisterTrackPiece(ATrackPiece* TrackPiece);

	/** Get world position of a lane at a given forward distance */
	UFUNCTION(BlueprintPure, Category = "Lane Spline")
	FVector GetLanePositionAtDistance(float Distance, int32 LaneIndex) const;

	/** Get world direction (tangent) of a lane at a given forward distance */
	UFUNCTION(BlueprintPure, Category = "Lane Spline")
	FVector GetLaneDirectionAtDistance(float Distance, int32 LaneIndex) const;

	/** Get total spline length for a lane */
	UFUNCTION(BlueprintPure, Category = "Lane Spline")
	float GetLaneSplineLength(int32 LaneIndex) const;

	/** Clear all splines (reset) */
	UFUNCTION(BlueprintCallable, Category = "Lane Spline")
	void ClearAllSplines();

protected:
	/** Spline points for each lane (stored as position + direction pairs) */
	struct FSplinePoint
	{
		FVector Position;
		FVector Direction;
		float Distance; // Cumulative distance along spline
		bool bSharpCorner;
	};

	/** Spline data for each lane (Left=0, Center=1, Right=2) */
	TArray<FSplinePoint> LaneSplines[3];

	/** Track pieces currently registered (for cleanup) */
	UPROPERTY()
	TArray<TWeakObjectPtr<ATrackPiece>> RegisteredTrackPieces;

	/** Get spline points for a lane index (0=Left, 1=Center, 2=Right) */
	TArray<FSplinePoint>& GetSplineByLaneIndex(int32 LaneIndex);
	const TArray<FSplinePoint>& GetSplineByLaneIndex(int32 LaneIndex) const;

	/** Add control point to a spline with proper continuity */
	void AddControlPointToSpline(TArray<FSplinePoint>& Spline, const FLaneControlPoint& ControlPoint, bool bIsEndPoint);

	/** Build smooth curve between two control points */
	void BuildSmoothCurve(TArray<FSplinePoint>& Spline, const FLaneControlPoint& StartPoint, const FLaneControlPoint& EndPoint);

	/** Add sharp corner point */
	void AddSharpCorner(TArray<FSplinePoint>& Spline, const FLaneControlPoint& CornerPoint);

	/** Evaluate spline position at distance using Catmull-Rom interpolation */
	FVector EvaluateSplineAtDistance(const TArray<FSplinePoint>& Spline, float Distance) const;

	/** Evaluate spline direction at distance */
	FVector EvaluateSplineDirectionAtDistance(const TArray<FSplinePoint>& Spline, float Distance) const;
};

