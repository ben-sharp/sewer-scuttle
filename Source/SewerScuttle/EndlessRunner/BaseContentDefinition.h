// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerClass.h"
#include "BaseContentDefinition.generated.h"

UENUM(BlueprintType)
enum class EContentType : uint8
{
	Coin,
	PowerUp,
	Obstacle,
	Unknown
};

UENUM(BlueprintType)
enum class ETrackTier : uint8
{
    T1	UMETA(DisplayName = "Tier 1"),
    T2	UMETA(DisplayName = "Tier 2"),
    T3	UMETA(DisplayName = "Tier 3")
};

namespace TrackTierUtils
{
    static FString ToString(ETrackTier Tier)
    {
        switch (Tier)
        {
            case ETrackTier::T2: return TEXT("T2");
            case ETrackTier::T3: return TEXT("T3");
            case ETrackTier::T1:
            default: return TEXT("T1");
        }
    }

    static ETrackTier FromString(const FString& TierStr)
    {
        if (TierStr.Equals(TEXT("T2"), ESearchCase::IgnoreCase)) return ETrackTier::T2;
        if (TierStr.Equals(TEXT("T3"), ESearchCase::IgnoreCase)) return ETrackTier::T3;
        return ETrackTier::T1;
    }
}

/** Base class for all spawnable content definitions (Obstacles, PowerUps, etc.) */
UCLASS(BlueprintType, Abstract)
class SEWERSCUTTLE_API UBaseContentDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Classes this content is allowed to appear for (empty = all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<EPlayerClass> AllowedClasses;

	/** Get the type of content this definition represents */
	virtual EContentType GetContentType() const { return EContentType::Unknown; }
};

/** Struct for weighted selection of a content definition */
USTRUCT(BlueprintType)
struct FWeightedDefinition
{
	GENERATED_BODY()

	/** The definition data asset (must be a subclass of BaseContentDefinition) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBaseContentDefinition* Definition = nullptr;

	/** Relative weight for selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

