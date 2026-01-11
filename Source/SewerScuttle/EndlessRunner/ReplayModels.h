// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerClass.h"
#include "ReplayModels.generated.h"

UENUM(BlueprintType)
enum class EReplayEventType : uint8
{
    MoveLeft,
    MoveRight,
    Jump,
    Slide,
    SlideReleased,
    PositionSync // Periodic position correction
};

USTRUCT(BlueprintType)
struct FReplayEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    EReplayEventType EventType = EReplayEventType::MoveLeft;

    /** Optional position for sync events or additional context */
	UPROPERTY(BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FLeaderboardEntryData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 RunId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Seed = 0;

	UPROPERTY(BlueprintReadWrite)
	FString SeedId;

    UPROPERTY(BlueprintReadWrite)
    EPlayerClass PlayerClass = EPlayerClass::Vanilla;

    UPROPERTY(BlueprintReadWrite)
    bool bHasReplay = false;
};
