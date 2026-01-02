// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TrackPiece.h"
#include "ObstacleSpawnComponent.generated.h"

/**
 * Component used in Track Piece blueprints to visually place obstacle spawn points
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UObstacleSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UObstacleSpawnComponent();

	/** Possible obstacles that can spawn here with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FWeightedDefinition> ObstacleDefinitions;

	/** Probability of ANY obstacle spawning at this point (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 1.0f;
};
