// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseContentDefinition.h"
#include "PlayerClass.h"
#include "CollectibleDefinition.generated.h"

/**
 * Defines properties for a collectible item
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UCollectibleDefinition : public UBaseContentDefinition
{
	GENERATED_BODY()

public:
	virtual EContentType GetContentType() const override { return EContentType::Coin; }

	/** Human-readable name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CollectibleName;

	/** The collectible blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSubclassOf<AActor> CollectibleClass;

	/** Value of this collectible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	int32 Value = 1;

	/** Is this a special collectible (for Collector class)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsSpecial = false;

	/** Multiplier for special collection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float SpecialValueMultiplier = 2.0f;

	/** Can this be attracted by magnets? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bMagnetable = true;

	/** Relative probability of being selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SelectionWeight = 1.0f;
};

