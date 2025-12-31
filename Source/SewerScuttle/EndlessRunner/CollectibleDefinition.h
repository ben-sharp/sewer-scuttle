// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"
#include "PlayerClass.h"
#include "CollectibleDefinition.generated.h"

/**
 * Data asset defining a collectible configuration
 * Used to define collectible properties for content export and server-side validation
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UCollectibleDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of this collectible definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	FString CollectibleName;

	/** Blueprint class to spawn (can be CollectibleCoin or MultiCollectible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Definition")
	TSubclassOf<AActor> CollectibleClass;

	/** Value of collectible (for single coins) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "1"))
	int32 Value = 1;

	/** Default item value (for multi-collectibles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "1"))
	int32 DefaultItemValue = 1;

	/** Whether this collectible can be attracted by magnet power-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bMagnetable = true;

	/** Whether this is a special collectible (for Collector class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsSpecial = false;

	/** Value multiplier for special collectibles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float SpecialValueMultiplier = 2.0f;

	/** Player classes that can encounter this collectible (empty = all classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	TArray<EPlayerClass> AllowedClasses;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection", meta = (ClampMin = "1"))
	int32 SelectionWeight = 1;
};

