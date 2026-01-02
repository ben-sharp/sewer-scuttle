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

