// Copyright Epic Games, Inc. All Rights Reserved.

#include "LaneSplineManager.h"
#include "TrackPiece.h"
#include "Math/UnrealMathUtility.h"

ALaneSplineManager::ALaneSplineManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALaneSplineManager::BeginPlay()
{
	Super::BeginPlay();
}

void ALaneSplineManager::RegisterTrackPiece(ATrackPiece* TrackPiece, const TArray<FTrackPieceLaneConfig>& LaneConfigs)
{
	if (!TrackPiece)
	{
		UE_LOG(LogTemp, Warning, TEXT("LaneSplineManager: Attempted to register null track piece"));
		return;
	}

	// Store reference to track piece
	RegisteredTrackPieces.Add(TrackPiece);

	// Register control points for each lane
	for (int32 LaneIndex = 0; LaneIndex < 3 && LaneIndex < LaneConfigs.Num(); ++LaneIndex)
	{
		TArray<FSplinePoint>& Spline = GetSplineByLaneIndex(LaneIndex);
		const FTrackPieceLaneConfig& Config = LaneConfigs[LaneIndex];

		// Add start point (if not the first piece, this should connect to previous)
		AddControlPointToSpline(Spline, Config.StartPoint, false);

		// Add end point
		AddControlPointToSpline(Spline, Config.EndPoint, true);

		UE_LOG(LogTemp, Log, TEXT("LaneSplineManager: Registered track piece '%s' lane %d - Start: (%.2f, %.2f, %.2f), End: (%.2f, %.2f, %.2f)"),
			*TrackPiece->GetName(), LaneIndex,
			Config.StartPoint.Position.X, Config.StartPoint.Position.Y, Config.StartPoint.Position.Z,
			Config.EndPoint.Position.X, Config.EndPoint.Position.Y, Config.EndPoint.Position.Z);
	}
}

void ALaneSplineManager::UnregisterTrackPiece(ATrackPiece* TrackPiece)
{
	if (!TrackPiece)
	{
		return;
	}

	// Remove from registered list
	RegisteredTrackPieces.RemoveAll([TrackPiece](const TWeakObjectPtr<ATrackPiece>& Ptr)
	{
		return Ptr.Get() == TrackPiece;
	});

	// Note: For now, we don't remove points from splines (would require tracking which points belong to which piece)
	// In a production system, you'd want to track point ownership and remove them
	// For now, splines will accumulate points - this is acceptable for an endless runner where pieces are rarely removed
	UE_LOG(LogTemp, Log, TEXT("LaneSplineManager: Unregistered track piece '%s'"), *TrackPiece->GetName());
}

FVector ALaneSplineManager::GetLanePositionAtDistance(float Distance, int32 LaneIndex) const
{
	const TArray<FSplinePoint>& Spline = GetSplineByLaneIndex(LaneIndex);
	if (Spline.Num() == 0)
	{
		// Fallback: return position based on distance along X axis
		float YOffset = 0.0f;
		switch (LaneIndex)
		{
		case 0: YOffset = -200.0f; break; // Left
		case 1: YOffset = 0.0f; break;     // Center
		case 2: YOffset = 200.0f; break; // Right
		}
		return FVector(Distance, YOffset, 0.0f);
	}

	// Clamp distance to spline length
	float SplineLength = Spline.Num() > 0 ? Spline.Last().Distance : 0.0f;
	float ClampedDistance = FMath::Clamp(Distance, 0.0f, SplineLength);

	// Get position along spline
	return EvaluateSplineAtDistance(Spline, ClampedDistance);
}

FVector ALaneSplineManager::GetLaneDirectionAtDistance(float Distance, int32 LaneIndex) const
{
	const TArray<FSplinePoint>& Spline = GetSplineByLaneIndex(LaneIndex);
	if (Spline.Num() == 0)
	{
		// Fallback: return forward direction
		return FVector(1.0f, 0.0f, 0.0f);
	}

	// Clamp distance to spline length
	float SplineLength = Spline.Num() > 0 ? Spline.Last().Distance : 0.0f;
	float ClampedDistance = FMath::Clamp(Distance, 0.0f, SplineLength);

	// Get direction (tangent) along spline
	return EvaluateSplineDirectionAtDistance(Spline, ClampedDistance);
}

float ALaneSplineManager::GetLaneSplineLength(int32 LaneIndex) const
{
	const TArray<FSplinePoint>& Spline = GetSplineByLaneIndex(LaneIndex);
	if (Spline.Num() == 0)
	{
		return 0.0f;
	}

	return Spline.Last().Distance;
}

void ALaneSplineManager::ClearAllSplines()
{
	LaneSplines[0].Empty();
	LaneSplines[1].Empty();
	LaneSplines[2].Empty();
	RegisteredTrackPieces.Empty();

	UE_LOG(LogTemp, Log, TEXT("LaneSplineManager: Cleared all splines"));
}

TArray<ALaneSplineManager::FSplinePoint>& ALaneSplineManager::GetSplineByLaneIndex(int32 LaneIndex)
{
	return LaneSplines[LaneIndex];
}

const TArray<ALaneSplineManager::FSplinePoint>& ALaneSplineManager::GetSplineByLaneIndex(int32 LaneIndex) const
{
	return LaneSplines[LaneIndex];
}

void ALaneSplineManager::AddControlPointToSpline(TArray<FSplinePoint>& Spline, const FLaneControlPoint& ControlPoint, bool bIsEndPoint)
{
	FSplinePoint NewPoint;
	NewPoint.Position = ControlPoint.Position;
	NewPoint.Direction = ControlPoint.Direction.GetSafeNormal();
	NewPoint.bSharpCorner = ControlPoint.bSharpCorner;

	// Calculate cumulative distance
	if (Spline.Num() > 0)
	{
		const FSplinePoint& LastPoint = Spline.Last();
		float SegmentDistance = FVector::Dist(LastPoint.Position, NewPoint.Position);
		NewPoint.Distance = LastPoint.Distance + SegmentDistance;
	}
	else
	{
		NewPoint.Distance = 0.0f;
	}

	Spline.Add(NewPoint);
}

void ALaneSplineManager::BuildSmoothCurve(TArray<FSplinePoint>& Spline, const FLaneControlPoint& StartPoint, const FLaneControlPoint& EndPoint)
{
	// Smooth curves are handled by linear interpolation between points
	// For more advanced curves, we could add intermediate control points here
}

void ALaneSplineManager::AddSharpCorner(TArray<FSplinePoint>& Spline, const FLaneControlPoint& CornerPoint)
{
	FSplinePoint NewPoint;
	NewPoint.Position = CornerPoint.Position;
	NewPoint.Direction = CornerPoint.Direction.GetSafeNormal();
	NewPoint.bSharpCorner = true;

	// Calculate cumulative distance
	if (Spline.Num() > 0)
	{
		const FSplinePoint& LastPoint = Spline.Last();
		float SegmentDistance = FVector::Dist(LastPoint.Position, NewPoint.Position);
		NewPoint.Distance = LastPoint.Distance + SegmentDistance;
	}
	else
	{
		NewPoint.Distance = 0.0f;
	}

	Spline.Add(NewPoint);
}

FVector ALaneSplineManager::EvaluateSplineAtDistance(const TArray<FSplinePoint>& Spline, float Distance) const
{
	if (Spline.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	// Find the segment containing this distance
	for (int32 i = 0; i < Spline.Num() - 1; ++i)
	{
		if (Distance >= Spline[i].Distance && Distance <= Spline[i + 1].Distance)
		{
			// Interpolate between these two points
			float SegmentStart = Spline[i].Distance;
			float SegmentEnd = Spline[i + 1].Distance;
			float SegmentLength = SegmentEnd - SegmentStart;
			
			if (SegmentLength <= 0.0f)
			{
				return Spline[i].Position;
			}

			float Alpha = (Distance - SegmentStart) / SegmentLength;
			
			// Use linear interpolation (can be upgraded to Catmull-Rom later)
			return FMath::Lerp(Spline[i].Position, Spline[i + 1].Position, Alpha);
		}
	}

	// Clamp to last point
	return Spline.Last().Position;
}

FVector ALaneSplineManager::EvaluateSplineDirectionAtDistance(const TArray<FSplinePoint>& Spline, float Distance) const
{
	if (Spline.Num() == 0)
	{
		return FVector(1.0f, 0.0f, 0.0f);
	}

	// Find the segment containing this distance
	for (int32 i = 0; i < Spline.Num() - 1; ++i)
	{
		if (Distance >= Spline[i].Distance && Distance <= Spline[i + 1].Distance)
		{
			// Interpolate direction between these two points
			float SegmentStart = Spline[i].Distance;
			float SegmentEnd = Spline[i + 1].Distance;
			float SegmentLength = SegmentEnd - SegmentStart;
			
			if (SegmentLength <= 0.0f)
			{
				return Spline[i].Direction;
			}

			float Alpha = (Distance - SegmentStart) / SegmentLength;
			
			// Interpolate direction
			FVector InterpolatedDir = FMath::Lerp(Spline[i].Direction, Spline[i + 1].Direction, Alpha);
			return InterpolatedDir.GetSafeNormal();
		}
	}

	// Return last point's direction
	return Spline.Last().Direction;
}
