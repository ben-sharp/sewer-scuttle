// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TrackPiece.h"
#include "PowerUpSpawnComponent.generated.h"

/**
 * Component used in Track Piece blueprints to visually place power-up spawn points
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API UPowerUpSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPowerUpSpawnComponent();

	/** Possible power-ups that can spawn here with weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FWeightedDefinition> PowerUpDefinitions;

	/** Probability of ANY power-up spawning at this point (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 0.5f;
};
