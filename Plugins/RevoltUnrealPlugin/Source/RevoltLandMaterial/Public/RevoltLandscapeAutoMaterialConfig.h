#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture.h"
#include "RevoltLandscapeAutoMaterialConfig.generated.h"

/**
 * Definition of a single biome layer for the auto material
 */
USTRUCT(BlueprintType)
struct REVOLTLANDMATERIAL_API FRevoltBiomeLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture* Albedo = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture* Normal = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UTexture* Roughness = nullptr; // Optional, can use scalar

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float RoughnessVal = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	float Tiling = 0.01f;
};

/**
 * Data Asset to configure a fixed 4-layer auto-material:
 * 1. Ground (Base)
 * 2. Mid-Height (e.g. Hills/Grass)
 * 3. High-Height (e.g. Snow/Mountain)
 * 4. Slope (e.g. Cliffs) - Overrides everything on steep angles
 */
UCLASS(BlueprintType)
class REVOLTLANDMATERIAL_API URevoltLandscapeAutoMaterialConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString MaterialName = "M_AutoLandscape";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SavePath = "/Game/Materials";

	// --- LAYERS ---

	/** The base layer that appears everywhere by default (e.g. Sand/Dirt) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1. Ground Layer")
	FRevoltBiomeLayer GroundLayer;

	/** The first height layer (e.g. Grass) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Mid Layer")
	FRevoltBiomeLayer MidLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Mid Layer")
	float MidStartHeight = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "2. Mid Layer")
	float MidFalloff = 500.0f;

	/** The high altitude layer (e.g. Snow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. High Layer")
	FRevoltBiomeLayer HighLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. High Layer")
	float HighStartHeight = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3. High Layer")
	float HighFalloff = 1000.0f;

	/** The slope layer (e.g. Cliffs) - Renders on steep terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Slope Layer")
	FRevoltBiomeLayer SlopeLayer;

	/** Angle dot product threshold (1.0 = Flat, 0.0 = Vertical). Lower value = Steeper start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Slope Layer", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SlopeThreshold = 0.7f; // around 45 degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "4. Slope Layer")
	float SlopeFalloff = 0.1f;

	/** Generate the material asset */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Generation")
	void GenerateMaterial();
};

