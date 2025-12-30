#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LandscapeStampBrush.h"
#include "RevoltLandGenConfig.generated.h"

UENUM(BlueprintType)
enum class EStampBlendMode : uint8
{
	Additive,
	Subtractive,
	Max,
	Min,
	AlphaBlend
};

USTRUCT(BlueprintType)
struct FLandGenStampLayer
{
	GENERATED_BODY()

	/** The heightmap texture to stamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	UTexture2D* StampData = nullptr;

	/** Toggle this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	bool Enabled = true;

	/** How the stamp blends with existing terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	EStampBlendMode BlendMode = EStampBlendMode::Additive;

	/** Target number of stamps to place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0"))
	int32 Frequency = 10;

	/** Minimum scale variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (ClampMin = "0.1"))
	float ScaleMin = 1.0f;

	/** Maximum scale variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform", meta = (ClampMin = "0.1"))
	float ScaleMax = 1.0f;

	/** Intensity of the stamp's effect on Z-axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	float HeightScale = 1.0f;

	/** Optional: If set, spawns this Actor instead of baking pixels. Defaults to ALandscapeStampBrush. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	TSubclassOf<AActor> StampActorClass = ALandscapeStampBrush::StaticClass();

	/** Randomize yaw (0-360) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	bool RotationRandom = true;

	/** Radius around the stamp where other objects (or stamps) cannot spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float CollisionRadius = 0.0f;
};

USTRUCT(BlueprintType)
struct FLandGenHeightmapLayer
{
	GENERATED_BODY()

	/** The heightmap texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	UTexture2D* HeightmapData = nullptr;

	/** Toggle this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	bool Enabled = true;

	/** How this heightmap blends with existing terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	EStampBlendMode BlendMode = EStampBlendMode::Additive;

	/** Intensity multiplier for this heightmap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap", meta = (ClampMin = "0.0"))
	float Intensity = 1.0f;

	/** If true, randomize the X/Y orientation of the heightmap each generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	bool bRandomizeOrientation = false;
};

USTRUCT(BlueprintType)
struct FLandGenObjectLayer
{
	GENERATED_BODY()

	/** The object to spawn (Blueprint Actor or Static Mesh) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object", meta = (AllowedClasses = "/Script/Engine.StaticMesh, /Script/Engine.Blueprint"))
	UObject* AssetToSpawn;

	/** Toggle this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object")
	bool Enabled = true;

	/** Instances per square kilometer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float Density = 100.0f;

    /** Maximum total number of instances to spawn for this layer (0 = unlimited/density only) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0"))
    int32 MaxCount = 0;

	/** Minimum scaling range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector ScaleMin = FVector(1.0f);

	/** Maximum scaling range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector ScaleMax = FVector(1.0f);

	/** Random yaw */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	bool RandomRotation = true;

	/** If true, scale uniformly across all axes (X=Y=Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	bool bUniformScale = false;

	/** Sink/raise object relative to ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	float ZOffset = 0.0f;

	/** If true, restrict placement to specific height ranges */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Constraints")
    bool bUseHeightConstraints = false;

    /** Minimum terrain height for placement (0.0 = lowest, 1.0 = highest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Constraints", meta = (EditCondition = "bUseHeightConstraints", ClampMin = "0.0", ClampMax = "1.0"))
    float MinHeight = 0.0f;

    /** Maximum terrain height for placement (0.0 = lowest, 1.0 = highest) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Constraints", meta = (EditCondition = "bUseHeightConstraints", ClampMin = "0.0", ClampMax = "1.0"))
    float MaxHeight = 1.0f;

    /** If true, objects will align their Up vector to the terrain slope */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
    bool bAlignToSurface = false;

    /** If true, restrict placement to specific slope angles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Constraints")
    bool bUseSlopeConstraints = false;

    /** Minimum slope angle in degrees (0 = flat, 90 = vertical) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Constraints", meta = (EditCondition = "bUseSlopeConstraints", ClampMin = "0.0", ClampMax = "90.0"))
    float MinSlope = 0.0f;

    /** Maximum slope angle in degrees (0 = flat, 90 = vertical) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Constraints", meta = (EditCondition = "bUseSlopeConstraints", ClampMin = "0.0", ClampMax = "90.0"))
    float MaxSlope = 45.0f;

	/** If true, terrain under the object will be flattened */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification")
    bool bFlattenTerrain = false;

    /** Radius of the flattened area around the object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification", meta = (EditCondition = "bFlattenTerrain", ClampMin = "10.0"))
    float FlattenRadius = 500.0f;

	/** Falloff distance for blending the flattened area back to terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Modification", meta = (EditCondition = "bFlattenTerrain", ClampMin = "10.0"))
	float FlattenFalloff = 500.0f;

	/** Radius around the object where other objects cannot spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float CollisionRadius = 0.0f;

	/** Distance from map edge to avoid placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution", meta = (ClampMin = "0.0"))
	float EdgeBuffer = 0.0f;

	/** If true, spawns as Hierarchical Instanced Static Mesh (HISM) for performance (Foliage-style). Requires AssetToSpawn to be a Static Mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bUseInstancing = false;
};

/**
 * Configuration data asset for land generation parameters.
 * This allows developers to create reusable presets and easily modify generation settings.
 */
UCLASS(BlueprintType, Category = "Landscape Generation")
class REVOLTLANDGEN_API URevoltLandGenConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URevoltLandGenConfig();

	// ========== GENERATION MODES ==========

	/** Defines the playable area or generation limits */
	UPROPERTY(EditAnywhere, Category = "General")
	FBox2D WorldBounds = FBox2D(FVector2D(-100000, -100000), FVector2D(100000, 100000));

	/** If false, terrain remains flat (Z=0) */
	UPROPERTY(EditAnywhere, Category = "Modes")
	bool bEnableHeightmapGeneration = true;

	/** If true, applies stamp brushes to modify the heightmap */
	UPROPERTY(EditAnywhere, Category = "Modes")
	bool bEnableStamps = true;

	/** If true, applies imported heightmap layers */
	UPROPERTY(EditAnywhere, Category = "Modes")
	bool bEnableImportedHeightmaps = false;

	/** If true, a water plane is generated at the specified height */
	UPROPERTY(EditAnywhere, Category = "Modes")
	bool bEnableWaterPlane = false;

	// ========== DIMENSIONS ==========

	/** Master material applied to the terrain */
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	UMaterialInterface* LandscapeMaterial;

	/** Material applied to the water plane */
	UPROPERTY(EditAnywhere, Category = "Dimensions", meta = (EditCondition = "bEnableWaterPlane"))
	UMaterialInterface* WaterMaterial;

	/** Z-Height of the water plane */
	UPROPERTY(EditAnywhere, Category = "Dimensions", meta = (EditCondition = "bEnableWaterPlane"))
	float WaterHeight = 0.0f;

	/** Scale multiplier for the water plane size relative to the landscape (1.0 = exact match, >1.0 = larger) */
	UPROPERTY(EditAnywhere, Category = "Dimensions", meta = (EditCondition = "bEnableWaterPlane", ClampMin = "1.0"))
	float WaterScaleMultiplier = 1.0f;

	/** Imported heightmap layers */
	UPROPERTY(EditAnywhere, Category = "Dimensions")
	TArray<FLandGenHeightmapLayer> HeightmapLayers;

	// ========== NOISE GENERATION ==========

	/** Overall scale of the noise pattern */
	UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "0.001", ClampMax = "10.0"))
	float NoiseScale = 1.0f;

	/** Multiplier for height values before clamping */
	UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "0.1", ClampMax = "50.0"))
	float HeightMultiplier = 15.0f;

	/** Number of noise octaves (layers) */
	UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "1", ClampMax = "8"))
	int32 Octaves = 4;

	/** How much each octave contributes to the final result */
	UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float Persistence = 0.5f;

	/** Frequency multiplier for each octave */
	UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float Lacunarity = 2.0f;

    /** Number of smoothing passes to apply after noise generation (0 = none) */
    UPROPERTY(EditAnywhere, Category = "Noise Generation", meta = (ClampMin = "0", ClampMax = "10"))
    int32 SmoothingIterations = 0;

	/** Offset applied to noise coordinates */
	UPROPERTY(EditAnywhere, Category = "Noise Generation")
	FVector2D NoiseOffset = FVector2D(0, 0);

	// ========== STAMPS ==========

	/** Layered stamp configuration */
	UPROPERTY(EditAnywhere, Category = "Stamps")
	TArray<FLandGenStampLayer> StampLayers;

	// ========== OBJECT POPULATION ==========

	/** Layered object population configuration */
	UPROPERTY(EditAnywhere, Category = "Object Population")
	TArray<FLandGenObjectLayer> ObjectLayers;

	// ========== VALIDATION ==========

	/** Validate configuration parameters */
	UFUNCTION(CallInEditor, Category = "Configuration")
	void ValidateConfig();

#if WITH_EDITOR
	/** Called when properties change in editor */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Get asset name for display */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
