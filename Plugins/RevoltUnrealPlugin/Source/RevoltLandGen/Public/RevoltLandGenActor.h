#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RevoltLandGenConfig.h"
#include "RevoltLandGenActor.generated.h"

UCLASS()
class REVOLTLANDGEN_API ARevoltLandGen : public AActor
{
	GENERATED_BODY()
	
public:	
	ARevoltLandGen();

#if WITH_EDITORONLY_DATA
	// Configuration asset for land generation parameters
	UPROPERTY(EditAnywhere, Category = "Landscape Generation")
	URevoltLandGenConfig* Config = nullptr;

	// Random Seed for procedural generation
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Seed")
	int32 Seed = 12345;

	// If true, a new random seed will be generated each time "Generate" is clicked
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Seed")
	bool bRandomizeSeed = false;

	// Number of landscape components in X direction
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Size")
	int32 ComponentCountX = 8;

	// Number of landscape components in Y direction
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Size")
	int32 ComponentCountY = 8;

	// Number of sections per component (1 or 2)
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Size")
	int32 SectionsPerComponent = 1;

	// Number of quads per section (e.g. 63)
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Size")
	int32 QuadsPerSection = 63;

	// Vertical scale factor in cm per height unit
	UPROPERTY(EditAnywhere, Category = "Landscape Generation|Size")
	float ZScale = 100.0f;
#endif

#if WITH_EDITOR
	// Generates only the heightmap/landscape
	UFUNCTION(CallInEditor, Category = "Generate Terrain")
	void GenerateLandscape();

	// Clears only the landscape (and baked stamps)
	UFUNCTION(CallInEditor, Category = "Generate Terrain")
	void ClearLandscape();

	// Spawns objects onto the existing landscape
	UFUNCTION(CallInEditor, Category = "Populate Assets")
	void PlaceAssets();

	// Clears only the spawned assets
	UFUNCTION(CallInEditor, Category = "Populate Assets")
	void ClearAssets();
#endif

private:
	// Helper to calculate noise value for a specific coordinate
	float CalculateNoise(float X, float Y, const URevoltLandGenConfig* InConfig, int32 InSeed);
};
