// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerClass.generated.h"

/**
 * Player class types
 */
UENUM(BlueprintType)
enum class EPlayerClass : uint8
{
	Vanilla		UMETA(DisplayName = "Vanilla"),
	Rogue		UMETA(DisplayName = "Rogue"),
	Enforcer	UMETA(DisplayName = "Enforcer"),
	Joker		UMETA(DisplayName = "Joker"),
	Scout		UMETA(DisplayName = "Scout"),
	Collector	UMETA(DisplayName = "Collector")
};

/**
 * Flexible class data structure that allows mix and match of abilities
 * Designed to be extensible for future classes
 */
USTRUCT(BlueprintType)
struct FPlayerClassData
{
	GENERATED_BODY()

	/** Extra lives to start with (default 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	int32 ExtraLives = 0;

	/** Maximum jump count (default 1 = single jump) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxJumpCount = 1;

	/** Capsule half height (default -1 means use normal size) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	float CapsuleHalfHeight = -1.0f;

	/** Never needs to crouch (can pass under low obstacles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bNeverNeedsCrouch = false;

	/** Can break through breakable obstacles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bCanBreakObstacles = false;

	/** Last legs score multiplier scales with speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bScalesLastLegsWithSpeed = false;

	/** Spawns special collectibles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bSpawnsSpecialCollectibles = false;

	/** Has starting magnet power-up (for future classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bHasStartingMagnet = false;

	/** Starting magnet duration in seconds (for future classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (ClampMin = "0.0", ClampMax = "300.0"))
	float StartingMagnetDuration = 0.0f;

	/** Base speed multiplier (for future classes, default 1.0 = normal speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float BaseSpeedMultiplier = 1.0f;

	/** Damage reduction multiplier (for future classes, default 1.0 = no reduction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageReduction = 1.0f;

	/** Get default class data for a specific class */
	static FPlayerClassData GetDefaultClassData(EPlayerClass Class);
};


