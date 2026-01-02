// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TrackPiece.h"
#include "CollectibleSpawnComponent.generated.h"

/**
 * Component used in Track Piece blueprints to visually place collectible (coin) spawn points
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UCollectibleSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCollectibleSpawnComponent();

	/** Possible collectibles that can spawn here with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FWeightedDefinition> CollectibleDefinitions;

	/** Probability of ANY collectible spawning at this point (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 1.0f;
};
