#include "RevoltLandGenActor.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Blueprint.h"
#include "Math/UnrealMathUtility.h"
#include "LandscapeStampBrush.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#include "EngineUtils.h"
#include "Containers/Array.h" // For TArray64

// Struct to hold planned object spawn data
struct FPlannedObject
{
    TSubclassOf<AActor> ActorClass;
    UStaticMesh* StaticMesh;
    FTransform Transform;
    bool bFlattenTerrain;
    float FlattenRadius;
    float FlattenFalloff;
    float ExpectedZ; // Calculated Z from HeightData
    float CollisionRadius; // Radius to block other objects
    float ZOffset; // Z offset to apply to final position
    bool bUseInstancing;

    // Slope settings
    bool bAlignToSurface = false;
    bool bUseSlopeConstraints = false;
    float MinSlope = 0.0f;
    float MaxSlope = 90.0f;
};

// Struct to hold planned stamp data
struct FPlannedStamp
{
    UTexture2D* StampTexture;
    FVector Location; // Grid Coordinates
    float Scale;
    float Rotation;
    float HeightScale;
    EStampBlendMode BlendMode;
    // We need these to spawn the actual actor later
    TSubclassOf<AActor> ActorClass;
    int32 LayerIndex; 
    int32 InstanceIndex;
    int32 Frequency;
    // Cached dimensions for optimization
    int32 TexW;
    int32 TexH;
};

ARevoltLandGen::ARevoltLandGen()
{
#if WITH_EDITORONLY_DATA
	bIsEditorOnlyActor = true;
#endif
}

float ARevoltLandGen::CalculateNoise(float X, float Y, const URevoltLandGenConfig* InConfig, int32 InSeed)
{
	if (!InConfig)
	{
		return 0.0f;
	}

	const int32 UseOctaves = InConfig->Octaves;
	const float UseNoiseScale = InConfig->NoiseScale;
	const float UsePersistence = InConfig->Persistence;
	const float UseLacunarity = InConfig->Lacunarity;
	const int32 UseSeed = InSeed; // Use passed-in seed
	const FVector2D UseNoiseOffset = InConfig->NoiseOffset;

	float NoiseValue = 0.0f;
	float Amplitude = 1.0f;
	float Frequency = 1.0f;
	float MaxValue = 0.0f;

	for (int32 i = 0; i < UseOctaves; i++)
	{
		float SeedOffsetX = (UseSeed % 100000) * 13.13f + (i * 100.0f); 
		float SeedOffsetY = (UseSeed % 100000) * 17.17f + (i * 100.0f);

		float BaseX = (X * Frequency * 0.01f * UseNoiseScale) + UseNoiseOffset.X + SeedOffsetX;
		float BaseY = (Y * Frequency * 0.01f * UseNoiseScale) + UseNoiseOffset.Y + SeedOffsetY;

		float NX = BaseX * 0.866f - BaseY * 0.5f;
		float NY = BaseX * 0.5f + BaseY * 0.866f;

		float OctaveNoise = FMath::PerlinNoise2D(FVector2D(NX, NY));
		NoiseValue += OctaveNoise * Amplitude;
		MaxValue += Amplitude;
		
		if (X == 0 && Y == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  Octave %d: Freq=%f, Amp=%f, Coord=(%f, %f), Result=%f"), i, Frequency, Amplitude, NX, NY, OctaveNoise);
		}

		Amplitude *= UsePersistence;
		Frequency *= UseLacunarity;
	}

	if (MaxValue > 0.0f)
	{
		return NoiseValue / MaxValue;
	}
	return 0.0f;
}

#if WITH_EDITOR

// Helper to extract normalized float data from a texture
static bool GetStampData(UTexture2D* Texture, TArray<float>& OutValues, int32& OutW, int32& OutH)
{
	if (!Texture) return false;

#if WITH_EDITOR
	FTextureSource& Source = Texture->Source;
	OutW = Source.GetSizeX();
	OutH = Source.GetSizeY();

	if (OutW <= 0 || OutH <= 0) 
    {
        UE_LOG(LogTemp, Warning, TEXT("LandGen: Texture source size is 0."));
		return false;
    }

    // Lock source mip 0
	TArray64<uint8> MipData;
	if (!Source.GetMipData(MipData, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("LandGen: Failed to get mip data from texture source."));
		return false;
	}

    const uint8* Data = MipData.GetData();
	OutValues.SetNum(OutW * OutH);

	ETextureSourceFormat Format = Source.GetFormat();
    
	UE_LOG(LogTemp, Log, TEXT("LandGen: Reading Stamp Texture '%s' (%dx%d) Format: %d"), *Texture->GetName(), OutW, OutH, (int32)Format);

    if (Format == ETextureSourceFormat::TSF_G8)
    {
		for (int32 i = 0; i < OutValues.Num(); i++)
		{
			OutValues[i] = Data[i] / 255.0f;
		}
    }
    else if (Format == ETextureSourceFormat::TSF_BGRA8)
    {
		const FColor* Pixels = (const FColor*)Data;
		for (int32 i = 0; i < OutValues.Num(); i++)
		{
			OutValues[i] = Pixels[i].R / 255.0f;
		}
    }
    else if (Format == ETextureSourceFormat::TSF_G16)
    {
		const uint16* Pixels = (const uint16*)Data;
		for (int32 i = 0; i < OutValues.Num(); i++)
		{
			OutValues[i] = Pixels[i] / 65535.0f;
		}
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("LandGen: Unsupported stamp texture source format: %d. Supported: BGRA8, G8, G16."), (int32)Format);
		for (int32 i = 0; i < OutValues.Num(); i++)
		{
			OutValues[i] = 0.0f;
		}
    }

	return true;
#else
    UE_LOG(LogTemp, Error, TEXT("LandGen: GetStampData only supported in Editor."));
    return false;
#endif
}

// Helper for simple 3x3 blur smoothing
static void SmoothHeightData(TArray<uint16>& Data, int32 W, int32 H, int32 Iterations)
{
    if (Iterations <= 0) return;
    
    TArray<uint16> Temp = Data; // Copy
    
    for (int32 iter = 0; iter < Iterations; iter++)
    {
        // Simple box blur
        for (int32 y = 0; y < H; y++)
        {
            for (int32 x = 0; x < W; x++)
            {
                uint32 Sum = 0;
                int32 Count = 0;
                
                // Kernel 3x3
                for (int32 ky = -1; ky <= 1; ky++)
                {
                    for (int32 kx = -1; kx <= 1; kx++)
                    {
                        int32 py = FMath::Clamp(y + ky, 0, H - 1);
                        int32 px = FMath::Clamp(x + kx, 0, W - 1);
                        
                        Sum += Temp[py * W + px];
                        Count++;
                    }
                }
                
                Data[y * W + x] = (uint16)(Sum / Count);
            }
        }
        // Update temp for next iteration
        Temp = Data;
    }
}

// Process a single planned stamp (Bake to heightmap)
static void ApplyPlannedStampToHeightData(
    const FPlannedStamp& Stamp,
    TArray<uint16>& HeightData,
    int32 MapW, int32 MapH,
    const TArray<float>& StampPixels,
    int32 StampW, int32 StampH)
{
    int32 DrawW = FMath::RoundToInt(Stamp.TexW * Stamp.Scale);
    int32 DrawH = FMath::RoundToInt(Stamp.TexH * Stamp.Scale);

    int32 StartX = FMath::RoundToInt(Stamp.Location.X) - DrawW / 2;
    int32 StartY = FMath::RoundToInt(Stamp.Location.Y) - DrawH / 2;

    for (int32 y = 0; y < DrawH; y++)
    {
        int32 MapY = StartY + y;
        if (MapY < 0 || MapY >= MapH) continue;

        for (int32 x = 0; x < DrawW; x++)
        {
            int32 MapX = StartX + x;
            if (MapX < 0 || MapX >= MapW) continue;

            float U = (float)x / (float)DrawW;
            float V = (float)y / (float)DrawH;

            int32 StampX = FMath::Clamp(FMath::FloorToInt(U * Stamp.TexW), 0, Stamp.TexW - 1);
            int32 StampY = FMath::Clamp(FMath::FloorToInt(V * Stamp.TexH), 0, Stamp.TexH - 1);
            
            float StampValue = StampPixels[StampY * Stamp.TexW + StampX];

            // Apply HeightScale (percentage of total Z)
            float AddedHeight = StampValue * (Stamp.HeightScale / 100.0f); 
            float HeightDelta = AddedHeight; 

            int32 Index = MapY * MapW + MapX;
            float CurrentHeight01 = HeightData[Index] / 65535.0f;
            
            float NewHeight01 = CurrentHeight01;

            if (Stamp.BlendMode == EStampBlendMode::Additive)
            {
                NewHeight01 += HeightDelta;
            }
            else if (Stamp.BlendMode == EStampBlendMode::Subtractive)
            {
                NewHeight01 -= HeightDelta;
            }
            else if (Stamp.BlendMode == EStampBlendMode::Max)
            {
                NewHeight01 = FMath::Max(NewHeight01, HeightDelta); 
            }
            
            NewHeight01 = FMath::Clamp(NewHeight01, 0.0f, 1.0f);
            HeightData[Index] = (uint16)(NewHeight01 * 65535.0f);
        }
    }
}

// Spawn a single planned stamp actor
static AActor* SpawnPlannedStampActor(
    const FPlannedStamp& Stamp,
    UWorld* World,
    ALandscape* Landscape,
    int32 TargetLayerIndex)
{
    if (!World || !Landscape) return nullptr;

    // Fallback logic for class
    UClass* SpawnClass = Stamp.ActorClass;
    if (SpawnClass == ALandscapeStampBrush::StaticClass())
    {
        UClass* BPClass = LoadClass<UObject>(nullptr, TEXT("/RevoltUnrealPlugin/BP_LandscapeStampBrush.BP_LandscapeStampBrush_C"));
        if (!BPClass) BPClass = LoadClass<UObject>(nullptr, TEXT("/Game/Data/Biomes/BP_LandscapeStampBrush.BP_LandscapeStampBrush_C"));
        
        if (BPClass) SpawnClass = BPClass;
    }

    FVector LandscapeLocation = Landscape->GetActorLocation();
    FVector LandscapeScale = Landscape->GetActorScale3D();

    FVector SpawnLoc = LandscapeLocation + FVector(Stamp.Location.X * LandscapeScale.X, Stamp.Location.Y * LandscapeScale.Y, 0.0f);
    SpawnLoc.Z = LandscapeLocation.Z; 

    FRotator SpawnRot(0, Stamp.Rotation, 0);
    FVector Scale3D(Stamp.Scale, Stamp.Scale, 1.0f);

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* NewActor = World->SpawnActor<AActor>(SpawnClass, SpawnLoc, SpawnRot, P);
    if (NewActor)
    {
        NewActor->SetActorScale3D(Scale3D);
        
#if WITH_EDITOR
        NewActor->SetActorLabel(FString::Printf(TEXT("Stamp_%d_%d"), Stamp.LayerIndex, Stamp.InstanceIndex));
        NewActor->SetFolderPath(FName("Stamps"));
#endif

        ALandscapeStampBrush* Brush = Cast<ALandscapeStampBrush>(NewActor);
        if (Brush)
        {
            Brush->StampTexture = Stamp.StampTexture;
            Brush->Intensity = Stamp.HeightScale;
            Brush->SetCanAffectHeightmap(true);
            
            if (TargetLayerIndex != -1)
            {
                Landscape->AddBrushToLayer(TargetLayerIndex, Brush);
            }
        }
    }
    return NewActor;
}

// Helper function to perform object planning without spawning (reused logic)
static void PlanObjectsAndStamps(
    ARevoltLandGen* Generator, 
    const URevoltLandGenConfig* Config,
    int32 Seed,
    int32 Width, int32 Height,
    const TArray<uint16>& HeightData,
    TArray<FPlannedObject>& OutPlannedObjects,
    TArray<FPlannedStamp>& OutPlannedStamps,
    TMap<UTexture2D*, TArray<float>>& OutTextureCache)
{
    // Track occupied locations for collision detection (X, Y, Z, Radius)
    TArray<FVector4> OccupiedLocations;

    // Check for existing objects in the world if we want persistent collision
    // Since "generation" and "placement" are separate buttons, we should ideally check existing actors if we are "Adding"
    // However, the current flow for "PlaceAssets" assumes a fresh pass based on seed.
    // If the user presses "PlaceAssets" again, they likely cleared first or expect a new seed layout.
    // If they want to respect EXISTING manual placements, we'd need to iterate actors.
    // For now, let's just make sure stamps and objects respect EACH OTHER in this planning phase.

    // Re-run Stamp Planning logic just to know where they are for height calculation
	if (Config && Config->bEnableStamps)
	{
		int32 LayerIndex = 0;
		for (const FLandGenStampLayer& Layer : Config->StampLayers)
		{
            int32 LayerSeed = Seed + (LayerIndex * 7919); 
            
			if (Layer.StampActorClass) 
            {
                if (Layer.Enabled && Layer.StampData)
                {
                    // Cache texture data
                    if (!OutTextureCache.Contains(Layer.StampData))
                    {
                        TArray<float> Pixels;
                        int32 TexW, TexH;
                        if (GetStampData(Layer.StampData, Pixels, TexW, TexH))
                        {
                            OutTextureCache.Add(Layer.StampData, Pixels);
                        }
                    }

                    FRandomStream StampStream(LayerSeed);
                    int32 StampCount = Layer.Frequency;
                    if (StampCount > 1000) StampCount = 1000;
                    
                    int32 TexW = 0, TexH = 0;
                    if (const TArray<float>* Pixels = OutTextureCache.Find(Layer.StampData))
                    {
                        TexW = Layer.StampData->Source.GetSizeX();
                        TexH = Layer.StampData->Source.GetSizeY();
                    }

                    if (TexW > 0 && TexH > 0)
                    {
                        for(int32 i=0; i<StampCount; i++)
                        {
                            int32 MaxAttempts = 10;
                            bool bFoundValidLocation = false;
                            float GridX = 0.0f;
                            float GridY = 0.0f;

                            for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
                            {
                                GridX = StampStream.FRandRange(0, Width - 1);
                                GridY = StampStream.FRandRange(0, Height - 1);

                                // Collision Check
                                // Note: Stamps use Grid Coordinates (0..Width), Objects use World Coordinates (0..Width*100)
                                // We need to convert Grid to World for consistent checks
                                float WorldX = GridX * 100.0f;
                                float WorldY = GridY * 100.0f;

                                if (Layer.CollisionRadius > 0.0f || OccupiedLocations.Num() > 0)
                                {
                                    bool bCollision = false;
                                    for (const FVector4& Occupied : OccupiedLocations)
                                    {
                                        float DistSq = FMath::Square(WorldX - Occupied.X) + FMath::Square(WorldY - Occupied.Y);
                                        // Occupied.W is the radius of the existing object
                                        float MinDist = Layer.CollisionRadius + Occupied.W;
                                        
                                        if (MinDist > 0.0f && DistSq < (MinDist * MinDist))
                                        {
                                            bCollision = true;
                                            break;
                                        }
                                    }

                                    if (bCollision)
                                    {
                                        continue; // Try next attempt
                                    }
                                }

                                bFoundValidLocation = true;
                                break;
                            }

                            if (!bFoundValidLocation) continue;

                            float Rotation = Layer.RotationRandom ? StampStream.FRandRange(0.0f, 360.0f) : 0.0f;
                            float Scale = StampStream.FRandRange(Layer.ScaleMin, Layer.ScaleMax);
                            
                            FPlannedStamp PS;
                            PS.StampTexture = Layer.StampData;
                            PS.Location = FVector(GridX, GridY, 0);
                            PS.Scale = Scale;
                            PS.Rotation = Rotation;
                            PS.HeightScale = Layer.HeightScale;
                            PS.BlendMode = Layer.BlendMode;
                            PS.ActorClass = Layer.StampActorClass;
                            PS.LayerIndex = LayerIndex;
                            PS.InstanceIndex = i;
                            PS.Frequency = Layer.Frequency;
                            PS.TexW = TexW;
                            PS.TexH = TexH;
                            
                            OutPlannedStamps.Add(PS);

                            // Add to Occupied Locations (Convert Grid to World coordinates for consistency)
                            if (Layer.CollisionRadius > 0.0f)
                            {
                                float WorldX = GridX * 100.0f;
                                float WorldY = GridY * 100.0f;
                                OccupiedLocations.Add(FVector4(WorldX, WorldY, 0.0f, Layer.CollisionRadius));
                            }
                        }
                    }
                }
            }
			LayerIndex++;
		}
	}
    int32 LayerCount = Config ? Config->ObjectLayers.Num() : 0;
    float ZScale = Generator ? Generator->ZScale : 100.0f;
    
    // Track occupied locations for collision detection (X, Y, Z, Radius)
    // TArray<FVector4> OccupiedLocations; // Already defined at start of function

    if (Config)
    {
        int32 LayerIndex = 0;
        for (const FLandGenObjectLayer& Layer : Config->ObjectLayers)
        {
            if (!Layer.Enabled || !Layer.AssetToSpawn)
            {
                LayerIndex++;
                continue;
            }

            FRandomStream Stream(Seed + (LayerIndex * 1337));
            
            float WorldWidth = (float)Width * 100.0f; 
            float WorldHeight = (float)Height * 100.0f;
            float AreaKm2 = (WorldWidth / 100000.0f) * (WorldHeight / 100000.0f);
            
            int32 Count = FMath::RoundToInt(Layer.Density * AreaKm2);
            if (Count <= 0) Count = 1;
            
            if (Layer.MaxCount > 0 && Count > Layer.MaxCount)
            {
                Count = Layer.MaxCount;
            }
            // Removed arbitrary 5000 limit, increased to 100000 for safety but allow large density
            if (Count > 100000) Count = 100000;

            UE_LOG(LogTemp, Log, TEXT("LandGen: Planning Layer %d (Density: %.2f) - Target Count: %d"), LayerIndex, Layer.Density, Count);

            // Determine Type
            TSubclassOf<AActor> SpawnClass = nullptr;
            UStaticMesh* SpawnMesh = nullptr;

            if (UBlueprint* BP = Cast<UBlueprint>(Layer.AssetToSpawn))
            {
                if (BP->GeneratedClass && BP->GeneratedClass->IsChildOf(AActor::StaticClass()))
                {
                    SpawnClass = *BP->GeneratedClass;
                }
            }
            else if (UClass* Class = Cast<UClass>(Layer.AssetToSpawn))
            {
                if (Class->IsChildOf(AActor::StaticClass()))
                {
                    SpawnClass = Class;
                }
            }
            else if (UStaticMesh* Mesh = Cast<UStaticMesh>(Layer.AssetToSpawn))
            {
                SpawnMesh = Mesh;
            }

            if (!SpawnClass && !SpawnMesh)
            {
                LayerIndex++;
                continue;
            }

            // Progress logging for very large counts only
            if (Count > 50000)
            {
                UE_LOG(LogTemp, Log, TEXT("LandGen: Processing large layer (%d items)..."), Count);
            }

            int32 SkippedCount = 0;
            int32 CollisionFailures = 0;
            int32 HeightFailures = 0;

            for (int32 i = 0; i < Count; i++)
            {
                // Only log progress for very large batches to keep logs clean
                if (i > 0 && i % 50000 == 0)
                {
                    UE_LOG(LogTemp, Log, TEXT("LandGen: Layer %d Progress: %d / %d"), LayerIndex, i, Count);
                }

                // Retry loop to find valid location
                int32 MaxAttempts = 10;
                bool bFoundValidLocation = false;
                float X = 0.f, Y = 0.f;

                // Edge Buffer constraints
                float MinX = Layer.EdgeBuffer;
                float MaxX = WorldWidth - Layer.EdgeBuffer;
                float MinY = Layer.EdgeBuffer;
                float MaxY = WorldHeight - Layer.EdgeBuffer;

                if (MaxX <= MinX || MaxY <= MinY)
                {
                    // Map is too small for this buffer
                    SkippedCount++;
                    continue; 
                }

                for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
                {
                    X = Stream.FRandRange(MinX, MaxX);
                    Y = Stream.FRandRange(MinY, MaxY);

                    // Collision Check
                    if (Layer.CollisionRadius > 0.0f || OccupiedLocations.Num() > 0)
                    {
                        bool bCollision = false;
                        for (const FVector4& Occupied : OccupiedLocations)
                        {
                            float DistSq = FMath::Square(X - Occupied.X) + FMath::Square(Y - Occupied.Y);
                            float MinDist = Layer.CollisionRadius + Occupied.W;
                            
                            // Optimization: Only check if radii are significant
                            if (MinDist > 0.0f && DistSq < (MinDist * MinDist))
                            {
                                bCollision = true;
                                break;
                            }
                        }

                        if (bCollision)
                        {
                            CollisionFailures++;
                            continue; // Try next attempt
                        }
                    }

                    // Height constraint check
                    if (Layer.bUseHeightConstraints)
                    {
                        // Calculate terrain height at this location
                        int32 GridX = FMath::Clamp(FMath::RoundToInt(X / 100.0f), 0, Width - 1);
                        int32 GridY = FMath::Clamp(FMath::RoundToInt(Y / 100.0f), 0, Height - 1);

                        float TerrainHeight = HeightData[GridY * Width + GridX] / 65535.0f;

                        // Check if height is within the specified range
                        if (TerrainHeight < Layer.MinHeight || TerrainHeight > Layer.MaxHeight)
                        {
                            HeightFailures++;
                            continue; // Try next attempt - height not in range
                        }
                    }

                    bFoundValidLocation = true;
                    break;
                }

                if (!bFoundValidLocation)
                {
                    SkippedCount++;
                    continue; // Skip this object if we couldn't find a spot
                }
                
                FPlannedObject Obj;
                Obj.ActorClass = SpawnClass;
                Obj.StaticMesh = SpawnMesh;
                Obj.bFlattenTerrain = Layer.bFlattenTerrain;
                Obj.FlattenRadius = Layer.FlattenRadius;
                Obj.FlattenFalloff = Layer.FlattenFalloff;
                Obj.CollisionRadius = Layer.CollisionRadius;
                Obj.ZOffset = Layer.ZOffset;
                Obj.bUseInstancing = Layer.bUseInstancing;
                
                // Pass slope settings
                Obj.bAlignToSurface = Layer.bAlignToSurface;
                Obj.bUseSlopeConstraints = Layer.bUseSlopeConstraints;
                Obj.MinSlope = Layer.MinSlope;
                Obj.MaxSlope = Layer.MaxSlope;

                // Handle Mesh extraction for instancing if it's a BP
                if (Layer.bUseInstancing && !SpawnMesh && SpawnClass)
                {
                    // Try to get mesh from CDO
                    AActor* CDO = Cast<AActor>(SpawnClass->GetDefaultObject());
                    if (CDO)
                    {
                        // Look for first static mesh component
                        UStaticMeshComponent* SMC = CDO->FindComponentByClass<UStaticMeshComponent>();
                        if (SMC) SpawnMesh = SMC->GetStaticMesh();
                    }
                }
                Obj.StaticMesh = SpawnMesh; // Ensure this is set for instancing
                
                FVector Scale;
                if (Layer.bUniformScale)
                {
                    // Use uniform scaling - pick one random value and apply to all axes
                    float UniformScale = Stream.FRandRange(Layer.ScaleMin.X, Layer.ScaleMax.X);
                    Scale = FVector(UniformScale, UniformScale, UniformScale);
                }
                else
                {
                    // Independent scaling per axis
                    Scale = FVector(
                        Stream.FRandRange(Layer.ScaleMin.X, Layer.ScaleMax.X),
                        Stream.FRandRange(Layer.ScaleMin.Y, Layer.ScaleMax.Y),
                        Stream.FRandRange(Layer.ScaleMin.Z, Layer.ScaleMax.Z)
                    );
                }
                
                FRotator Rotation = FRotator::ZeroRotator;
                if (Layer.RandomRotation)
                {
                    Rotation.Yaw = Stream.FRandRange(0.0f, 360.0f);
                }
                
                // Removed AlignToSurface logic as it was unused/unimplemented
                
                FVector Loc(X, Y, 0.0f);
                Obj.Transform = FTransform(Rotation, Loc, Scale);
                
                // Calculate Expected Z
                int32 GridX = FMath::Clamp(FMath::RoundToInt(Loc.X / 100.0f), 0, Width - 1);
                int32 GridY = FMath::Clamp(FMath::RoundToInt(Loc.Y / 100.0f), 0, Height - 1);
                
                float CurrentHeight01 = HeightData[GridY * Width + GridX] / 65535.0f;
                
                for (const FPlannedStamp& PS : OutPlannedStamps)
                {
                    int32 DrawW = FMath::RoundToInt(PS.TexW * PS.Scale);
                    int32 DrawH = FMath::RoundToInt(PS.TexH * PS.Scale);
                    int32 StartX = FMath::RoundToInt(PS.Location.X) - DrawW / 2;
                    int32 StartY = FMath::RoundToInt(PS.Location.Y) - DrawH / 2;
                    
                    if (GridX >= StartX && GridX < StartX + DrawW &&
                        GridY >= StartY && GridY < StartY + DrawH)
                    {
                        int32 LocalX = GridX - StartX;
                        int32 LocalY = GridY - StartY;
                        
                        float U = (float)LocalX / (float)DrawW;
                        float V = (float)LocalY / (float)DrawH;
                        
                        int32 TexX = FMath::Clamp(FMath::FloorToInt(U * PS.TexW), 0, PS.TexW - 1);
                        int32 TexY = FMath::Clamp(FMath::FloorToInt(V * PS.TexH), 0, PS.TexH - 1);
                        
                        if (const TArray<float>* Pixels = OutTextureCache.Find(PS.StampTexture))
                        {
                            if (Pixels->IsValidIndex(TexY * PS.TexW + TexX))
                            {
                                float StampValue = (*Pixels)[TexY * PS.TexW + TexX];
                                
                                // Treat HeightScale as World Units (assuming user config is large, e.g. 5000)
                                // Normalized = Units / (512.0 * ZScale)
                                float AddedHeight = (StampValue * PS.HeightScale) / (512.0f * ZScale);
                                
                                if (PS.BlendMode == EStampBlendMode::Additive)
                                    CurrentHeight01 += AddedHeight;
                                else if (PS.BlendMode == EStampBlendMode::Subtractive)
                                    CurrentHeight01 -= AddedHeight;
                                else if (PS.BlendMode == EStampBlendMode::Max)
                                    CurrentHeight01 = FMath::Max(CurrentHeight01, AddedHeight);
                                
                                CurrentHeight01 = FMath::Clamp(CurrentHeight01, 0.0f, 1.0f);
                            }
                        }
                    }
                }

                Obj.ExpectedZ = CurrentHeight01;
                OutPlannedObjects.Add(Obj);

                // Register occupancy if this object has a collision radius OR if we want to block future objects from hitting this one
                // Even if Radius is 0, we might want to register it with a small radius? 
                // For now, only register if CollisionRadius > 0 to save perf, or if other objects need to avoid it.
                // The user asked "allow other objects to be placed near it". 
                // So if Object A has radius 500, Object B (radius 0) checks against A (500+0).
                // If Object A has radius 0, Object B (radius 500) checks against A (0+500).
                // So we should ALWAYS add it to OccupiedLocations, using its Config Radius.
                if (Layer.CollisionRadius > 0.0f)
                {
                    OccupiedLocations.Add(FVector4(X, Y, Loc.Z, Layer.CollisionRadius));
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("LandGen: Layer %d Complete. Planned: %d, Placed: %d, Skipped: %d"), LayerIndex, Count, Count - SkippedCount, SkippedCount);
            if (SkippedCount > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Failures -> Collision: %d, Height: %d"), CollisionFailures, HeightFailures);
            }

            LayerIndex++;
        }
    }
}

void ARevoltLandGen::GenerateLandscape()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("LandGen: Failed to get world context"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LandGen: Starting Generation..."));

	if (!Config)
	{
		UE_LOG(LogTemp, Error, TEXT("LandGen: No Config assigned! Generation aborted."));
        return;
	}

	Config->ValidateConfig();
	UE_LOG(LogTemp, Log, TEXT("LandGen: Using config: %s"), *Config->GetName());

	if (bRandomizeSeed)
	{
		Seed = FMath::Rand();
		UE_LOG(LogTemp, Log, TEXT("LandGen: Randomized Seed: %d"), Seed);
	}

	TArray<uint16> HeightData;
    
    const bool bGenerateHeightmap = Config->bEnableHeightmapGeneration;
    const float UseZScale = ZScale;
    const float UseHeightMultiplier = Config->HeightMultiplier;

    const int32 UseComponentCountX = ComponentCountX;
    const int32 UseComponentCountY = ComponentCountY;
    const int32 UseSectionsPerComponent = SectionsPerComponent;
    const int32 UseQuadsPerSection = QuadsPerSection;

    const int32 QuadsPerComponent = UseSectionsPerComponent * UseQuadsPerSection;
    const int32 TotalQuadsX = UseComponentCountX * QuadsPerComponent;
    const int32 TotalQuadsY = UseComponentCountY * QuadsPerComponent;

    const int32 Width = TotalQuadsX + 1;
    const int32 Height = TotalQuadsY + 1;

    // --- PHASE 0: Generate Base Heightmap (Heightmaps + Noise) ---
    HeightData.SetNum(Width * Height);

    // Use a temporary float array for heightmap processing
    TArray<float> FloatHeightData;
    FloatHeightData.SetNum(Width * Height);
    FMemory::Memset(FloatHeightData.GetData(), 0, FloatHeightData.Num() * sizeof(float));

    // Process heightmap layers
    if (Config->bEnableImportedHeightmaps && Config->HeightmapLayers.Num() > 0)
    {
        int32 LayerIndex = 0;
        for (const FLandGenHeightmapLayer& Layer : Config->HeightmapLayers)
        {
            if (!Layer.Enabled || !Layer.HeightmapData) 
            {
                LayerIndex++;
                continue;
            }

            TArray<float> HeightmapPixels;
            int32 HMW = 0, HMH = 0;

            if (GetStampData(Layer.HeightmapData, HeightmapPixels, HMW, HMH))
            {
                // Determine orientation based on random seed
                bool bFlipX = false;
                bool bFlipY = false;
                bool bSwapXY = false;

                if (Layer.bRandomizeOrientation)
                {
                    // Use a seed specific to this layer
                    FRandomStream OrientationStream(Seed + (LayerIndex * 541));
                    int32 Orientation = OrientationStream.RandRange(0, 3);
                    
                    // 0: Normal
                    // 1: Rot 90 (Swap XY, Flip X)
                    // 2: Rot 180 (Flip X, Flip Y)
                    // 3: Rot 270 (Swap XY, Flip Y)
                    // Or simpler: Random flip X, Random flip Y, Random Swap XY?
                    // Let's implement 8-way symmetry (Flip X, Flip Y, Swap XY)
                    
                    bFlipX = OrientationStream.RandRange(0, 1) == 1;
                    bFlipY = OrientationStream.RandRange(0, 1) == 1;
                    bSwapXY = OrientationStream.RandRange(0, 1) == 1;
                    
                    UE_LOG(LogTemp, Log, TEXT("LandGen: Randomized Heightmap Layer %d Orientation: FlipX=%d, FlipY=%d, SwapXY=%d"), 
                        LayerIndex, bFlipX, bFlipY, bSwapXY);
                }

                // Apply this heightmap layer to the float height data
                for (int32 Y = 0; Y < Height; Y++)
                {
                    for (int32 X = 0; X < Width; X++)
                    {
                        float U = (float)X / (float)(Width - 1);
                        float V = (float)Y / (float)(Height - 1);

                        // Apply transformations to UVs
                        float SourceU = U;
                        float SourceV = V;

                        if (bSwapXY)
                        {
                            float Temp = SourceU;
                            SourceU = SourceV;
                            SourceV = Temp;
                        }

                        if (bFlipX) SourceU = 1.0f - SourceU;
                        if (bFlipY) SourceV = 1.0f - SourceV;

                        int32 HMX = FMath::Clamp(FMath::FloorToInt(SourceU * HMW), 0, HMW - 1);
                        int32 HMY = FMath::Clamp(FMath::FloorToInt(SourceV * HMH), 0, HMH - 1);

                        float HeightmapValue = HeightmapPixels[HMY * HMW + HMX] * Layer.Intensity;
                        float& CurrentHeight = FloatHeightData[Y * Width + X];

                        // Apply blend mode for this layer
                        if (Layer.BlendMode == EStampBlendMode::Additive)
                            CurrentHeight += HeightmapValue;
                        else if (Layer.BlendMode == EStampBlendMode::Subtractive)
                            CurrentHeight -= HeightmapValue;
                        else if (Layer.BlendMode == EStampBlendMode::Max)
                            CurrentHeight = FMath::Max(CurrentHeight, HeightmapValue);
                        else if (Layer.BlendMode == EStampBlendMode::Min)
                            CurrentHeight = FMath::Min(CurrentHeight, HeightmapValue);
                        else if (Layer.BlendMode == EStampBlendMode::AlphaBlend)
                            CurrentHeight = HeightmapValue; // Replace mode

                        // Clamp to valid range
                        CurrentHeight = FMath::Clamp(CurrentHeight, 0.0f, 1.0f);
                    }
                }
            }
            LayerIndex++;
        }
    }

    // Apply noise generation to the float height data
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            float& Normalized = FloatHeightData[Y * Width + X];

            if (bGenerateHeightmap)
            {
                float Noise = CalculateNoise((float)X, (float)Y, Config, Seed);
                float Noise01 = (Noise + 1.0f) * 0.5f;
                Noise01 = FMath::Pow(Noise01, 1.5f);
                Noise01 *= UseHeightMultiplier;
                Noise01 = FMath::Clamp(Noise01, 0.0f, 1.0f);

                // Blend noise with existing height (from heightmaps)
                if (Config->bEnableImportedHeightmaps && Config->HeightmapLayers.Num() > 0)
                {
                    // For backward compatibility, use additive blending with noise
                    Normalized += Noise01;
                }
                else
                {
                    Normalized = Noise01;
                }

                Normalized = FMath::Clamp(Normalized, 0.0f, 1.0f);
            }
        }
    }

    // Convert float height data to uint16
    for (int32 i = 0; i < FloatHeightData.Num(); i++)
    {
        HeightData[i] = (uint16)(FloatHeightData[i] * 65535.0f);
    }

    if (Config->SmoothingIterations > 0)
    {
        SmoothHeightData(HeightData, Width, Height, Config->SmoothingIterations);
    }

    // --- PHASE 1: Plan Objects and Stamps ---
    TArray<FPlannedObject> PlannedObjects;
    TArray<FPlannedStamp> PlannedStamps;
    TMap<UTexture2D*, TArray<float>> TextureCache;
    
    // Run shared planning logic
    PlanObjectsAndStamps(this, Config, Seed, Width, Height, HeightData, PlannedObjects, PlannedStamps, TextureCache);

	// Apply Baked Stamps (Using Planned Locations)
	if (Config->bEnableStamps)
	{
		for (const FPlannedStamp& Stamp : PlannedStamps)
		{
            // Only bake stamps that have a texture (Mode 2 fallback if ActorClass is null or not spawned yet)
            // But ProcessStampLayer logic was: "Mode 2: Bake Pixels ... if (!Layer.StampActorClass)"
            // PlanObjectsAndStamps handles both.
            // If it has ActorClass, we spawn it later.
            // If it DOES NOT have ActorClass, we bake it now.
            
            if (!Stamp.ActorClass && Stamp.StampTexture)
            {
                TArray<float> Pixels;
                if (const TArray<float>* Cached = TextureCache.Find(Stamp.StampTexture))
                {
                    Pixels = *Cached;
                }
                else
                {
                    // Should be in cache from planning, but safe fallback
                    int32 TW, TH;
                    if (GetStampData(Stamp.StampTexture, Pixels, TW, TH))
                    {
                        TextureCache.Add(Stamp.StampTexture, Pixels);
                    }
                }
                
                if (Pixels.Num() > 0)
                {
                    ApplyPlannedStampToHeightData(Stamp, HeightData, Width, Height, Pixels, Stamp.TexW, Stamp.TexH);
                }
            }
		}
	}

    // --- PHASE 1.5: Apply Flattening to HeightData ---
    for (const FPlannedObject& Obj : PlannedObjects)
    {
        if (!Obj.bFlattenTerrain) continue;
        
        FVector Loc = Obj.Transform.GetLocation();
        int32 CenterX = FMath::RoundToInt(Loc.X / 100.0f);
        int32 CenterY = FMath::RoundToInt(Loc.Y / 100.0f);
        float TargetHeight = Obj.ExpectedZ;
        
        int32 RadiusGrid = FMath::CeilToInt(Obj.FlattenRadius / 100.0f);
        int32 FalloffGrid = FMath::CeilToInt(Obj.FlattenFalloff / 100.0f);
        int32 TotalRadius = RadiusGrid + FalloffGrid;
        
        for (int32 y = -TotalRadius; y <= TotalRadius; y++)
        {
            int32 MapY = CenterY + y;
            if (MapY < 0 || MapY >= Height) continue;
            
            for (int32 x = -TotalRadius; x <= TotalRadius; x++)
            {
                int32 MapX = CenterX + x;
                if (MapX < 0 || MapX >= Width) continue;
                
                float Dist = FMath::Sqrt((float)(x*x + y*y));
                if (Dist > TotalRadius) continue;
                
                float CurrentH = HeightData[MapY * Width + MapX] / 65535.0f;
                float NewH = CurrentH;
                
                // Calculate Flattening Alpha
                float FlattenAlpha = 0.0f;
                
                if (Dist <= RadiusGrid)
                {
                    FlattenAlpha = 1.0f;
                }
                else
                {
                    float Alpha = (Dist - RadiusGrid) / (float)FalloffGrid;
                    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
                    Alpha = Alpha * Alpha * (3 - 2 * Alpha);
                    FlattenAlpha = 1.0f - Alpha; // 1.0 at radius, 0.0 at falloff edge
                }
                
                // Calculate Expected Stamp Contribution at this point (Simulate Brushes)
                float TotalStampContribution = 0.0f;
                
                for (const FPlannedStamp& PS : PlannedStamps)
                {
                    int32 DrawW = FMath::RoundToInt(PS.TexW * PS.Scale);
                    int32 DrawH = FMath::RoundToInt(PS.TexH * PS.Scale);
                    int32 StartX = FMath::RoundToInt(PS.Location.X) - DrawW / 2;
                    int32 StartY = FMath::RoundToInt(PS.Location.Y) - DrawH / 2;
                    
                    if (MapX >= StartX && MapX < StartX + DrawW &&
                        MapY >= StartY && MapY < StartY + DrawH)
                    {
                        int32 LocalX = MapX - StartX;
                        int32 LocalY = MapY - StartY;
                        
                        float U = (float)LocalX / (float)DrawW;
                        float V = (float)LocalY / (float)DrawH;
                        
                        int32 TexX = FMath::Clamp(FMath::FloorToInt(U * PS.TexW), 0, PS.TexW - 1);
                        int32 TexY = FMath::Clamp(FMath::FloorToInt(V * PS.TexH), 0, PS.TexH - 1);
                        
                        if (const TArray<float>* Pixels = TextureCache.Find(PS.StampTexture))
                        {
                            if (Pixels->IsValidIndex(TexY * PS.TexW + TexX))
                            {
                                float StampValue = (*Pixels)[TexY * PS.TexW + TexX];
                                float Added = (StampValue * PS.HeightScale) / (512.0f * UseZScale);
                                
                                if (PS.BlendMode == EStampBlendMode::Additive) TotalStampContribution += Added;
                                else if (PS.BlendMode == EStampBlendMode::Subtractive) TotalStampContribution -= Added;
                                // Max blend is hard to inverse without knowing the order, assuming additive for flattening
                            }
                        }
                    }
                }
                
                // Target: We want (Base + Stamps) = TargetHeight
                // Base = TargetHeight - Stamps
                float DesiredBase = TargetHeight - TotalStampContribution;
                
                // Blend between CurrentBase and DesiredBase
                NewH = FMath::Lerp(CurrentH, DesiredBase, FlattenAlpha);
                NewH = FMath::Clamp(NewH, 0.0f, 1.0f);
                
                HeightData[MapY * Width + MapX] = (uint16)(NewH * 65535.0f);
            }
        }
    }

    // Spawn Landscape
	const FGuid BaseLayerGuid = FGuid();
	TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	HeightmapDataPerLayers.Add(BaseLayerGuid, HeightData);

	TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	MaterialLayerDataPerLayers.Add(BaseLayerGuid, MoveTemp(MaterialImportLayers));

	const int32 MinX = 0;
	const int32 MinY = 0;
	const int32 MaxX = TotalQuadsX;
	const int32 MaxY = TotalQuadsY;
    
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = GetLevel();
	ALandscape* Landscape = World->SpawnActor<ALandscape>(GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
    
    if (!Landscape) return;

    TArray<FLandscapeLayer> EmptyLayers;

	Landscape->Import(
		FGuid::NewGuid(),
		MinX, MinY, MaxX, MaxY,
		UseSectionsPerComponent,
		UseQuadsPerSection,
		HeightmapDataPerLayers,
		nullptr,
		MaterialLayerDataPerLayers,
		ELandscapeImportAlphamapType::Additive,
		EmptyLayers
	);

	Landscape->SetActorScale3D(FVector(100.0f, 100.0f, UseZScale));
	if (Config->LandscapeMaterial)
	{
		Landscape->LandscapeMaterial = Config->LandscapeMaterial;
		FPropertyChangedEvent MaterialPropertyChangedEvent(FindFieldChecked<FProperty>(Landscape->GetClass(), FName("LandscapeMaterial")));
		Landscape->PostEditChangeProperty(MaterialPropertyChangedEvent);
	}

	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (LandscapeInfo)
	{
		LandscapeInfo->UpdateLayerInfoMap(Landscape);
	}
	
	Landscape->RegisterAllComponents();
	Landscape->PostEditChange();

	UE_LOG(LogTemp, Log, TEXT("LandGen: Finished. Landscape Actor: %s"), *Landscape->GetName());

    // Apply Water Plane
    if (Config->bEnableWaterPlane)
    {
        FVector WaterLoc = GetActorLocation();
        WaterLoc.Z += Config->WaterHeight * UseZScale; // Scale Z height 
        
        FActorSpawnParameters WaterSpawnParams;
        WaterSpawnParams.OverrideLevel = GetLevel();
        
        AStaticMeshActor* WaterPlane = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), WaterLoc, FRotator::ZeroRotator, WaterSpawnParams);
        
        if (WaterPlane)
        {
            // Set mesh to Engine Plane
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
            if (PlaneMesh)
            {
                WaterPlane->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
            }
            
            // Set material
            if (Config->WaterMaterial)
            {
                WaterPlane->GetStaticMeshComponent()->SetMaterial(0, Config->WaterMaterial);
            }
            
            // Scale to cover the landscape
            // Plane is 100x100. Landscape size is Width * 100.
            // Scale = (Width * 100) / 100 = Width.
            // Apply WaterScaleMultiplier to extend beyond landscape bounds.
            float WaterScaleX = (float)Width * Config->WaterScaleMultiplier;
            float WaterScaleY = (float)Height * Config->WaterScaleMultiplier;
            
            // Center the water plane
            // Landscape starts at 0,0. Water plane pivot is center.
            // Center is Width*100 / 2, Height*100 / 2.
            FVector CenterLoc = WaterLoc;
            CenterLoc.X += (Width * 100.0f) * 0.5f;
            CenterLoc.Y += (Height * 100.0f) * 0.5f;
            
            WaterPlane->SetActorLocation(CenterLoc);
            WaterPlane->SetActorScale3D(FVector(WaterScaleX, WaterScaleY, 1.0f));
            
#if WITH_EDITOR
            WaterPlane->SetActorLabel(TEXT("Generated_WaterPlane"));
            WaterPlane->SetFolderPath(FName("Water"));
#endif
        }
    }

	// Pass 2: Spawn Stamp Actors (Using Planned Locations)
	if (Config->bEnableStamps)
	{
        int32 TargetLayerIndex = -1;
        TargetLayerIndex = Landscape->CreateLayer(FName("Stamps"));

		for (const FPlannedStamp& Stamp : PlannedStamps)
		{
			if (Stamp.ActorClass)
			{
                SpawnPlannedStampActor(Stamp, World, Landscape, TargetLayerIndex);
			}
		}
        
        if (TargetLayerIndex != -1)
        {
             Landscape->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_All);
        }
	}

	UE_LOG(LogTemp, Log, TEXT("LandGen: Landscape generation completed successfully"));
}

void ARevoltLandGen::PlaceAssets()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("LandGen: Failed to get world context"));
		return;
	}

	if (!Config)
	{
		UE_LOG(LogTemp, Error, TEXT("LandGen: No Config assigned! Cannot place assets."));
		return;
	}

    UE_LOG(LogTemp, Log, TEXT("LandGen: Placing Assets (Seed: %d)..."), Seed);

    // 1. Find existing Landscape to get location/transform
    ALandscape* Landscape = nullptr;
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
        Landscape = *It;
        break;
    }
    
    if (!Landscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("LandGen: No Landscape found. Cannot place assets."));
        return;
    }

    // 2. Reconstruct basic HeightData state (Noise + Baked) just for planning logic reference
    
    const int32 UseComponentCountX = ComponentCountX;
    const int32 UseComponentCountY = ComponentCountY;
    const int32 UseSectionsPerComponent = SectionsPerComponent;
    const int32 UseQuadsPerSection = QuadsPerSection;
    const int32 QuadsPerComponent = UseSectionsPerComponent * UseQuadsPerSection;
    const int32 TotalQuadsX = UseComponentCountX * QuadsPerComponent;
    const int32 TotalQuadsY = UseComponentCountY * QuadsPerComponent;
    const int32 Width = TotalQuadsX + 1;
    const int32 Height = TotalQuadsY + 1;

    TArray<uint16> HeightData;
    HeightData.SetNum(Width * Height);
    
    // Re-run Phase 0 (Height Gen) exactly as GenerateLandscape does
    // Use a temporary float array for heightmap processing
    TArray<float> FloatHeightData;
    FloatHeightData.SetNum(Width * Height);
    FMemory::Memset(FloatHeightData.GetData(), 0, FloatHeightData.Num() * sizeof(float));

    // Process heightmap layers
    if (Config->bEnableImportedHeightmaps && Config->HeightmapLayers.Num() > 0)
    {
        int32 LayerIndex = 0;
        for (const FLandGenHeightmapLayer& Layer : Config->HeightmapLayers)
        {
            if (!Layer.Enabled || !Layer.HeightmapData) 
            {
                LayerIndex++;
                continue;
            }

            TArray<float> HeightmapPixels;
            int32 HMW = 0, HMH = 0;

            if (GetStampData(Layer.HeightmapData, HeightmapPixels, HMW, HMH))
            {
                // Determine orientation based on random seed (MUST MATCH GENERATION SEED)
                bool bFlipX = false;
                bool bFlipY = false;
                bool bSwapXY = false;

                if (Layer.bRandomizeOrientation)
                {
                    // Use a seed specific to this layer
                    FRandomStream OrientationStream(Seed + (LayerIndex * 541));
                    int32 Orientation = OrientationStream.RandRange(0, 3);
                    
                    bFlipX = OrientationStream.RandRange(0, 1) == 1;
                    bFlipY = OrientationStream.RandRange(0, 1) == 1;
                    bSwapXY = OrientationStream.RandRange(0, 1) == 1;
                }

                // Apply this heightmap layer to the float height data
                for (int32 Y = 0; Y < Height; Y++)
                {
                    for (int32 X = 0; X < Width; X++)
                    {
                        float U = (float)X / (float)(Width - 1);
                        float V = (float)Y / (float)(Height - 1);

                        // Apply transformations to UVs
                        float SourceU = U;
                        float SourceV = V;

                        if (bSwapXY)
                        {
                            float Temp = SourceU;
                            SourceU = SourceV;
                            SourceV = Temp;
                        }

                        if (bFlipX) SourceU = 1.0f - SourceU;
                        if (bFlipY) SourceV = 1.0f - SourceV;

                        int32 HMX = FMath::Clamp(FMath::FloorToInt(SourceU * HMW), 0, HMW - 1);
                        int32 HMY = FMath::Clamp(FMath::FloorToInt(SourceV * HMH), 0, HMH - 1);

                        float HeightmapValue = HeightmapPixels[HMY * HMW + HMX] * Layer.Intensity;
                        float& CurrentHeight = FloatHeightData[Y * Width + X];

                        if (Layer.BlendMode == EStampBlendMode::Additive)
                            CurrentHeight += HeightmapValue;
                        else if (Layer.BlendMode == EStampBlendMode::Subtractive)
                            CurrentHeight -= HeightmapValue;
                        else if (Layer.BlendMode == EStampBlendMode::Max)
                            CurrentHeight = FMath::Max(CurrentHeight, HeightmapValue);
                        else if (Layer.BlendMode == EStampBlendMode::Min)
                            CurrentHeight = FMath::Min(CurrentHeight, HeightmapValue);
                        else if (Layer.BlendMode == EStampBlendMode::AlphaBlend)
                            CurrentHeight = HeightmapValue;

                        CurrentHeight = FMath::Clamp(CurrentHeight, 0.0f, 1.0f);
                    }
                }
            }
            LayerIndex++;
        }
    }

    const float UseHeightMultiplier = Config->HeightMultiplier;

    // Apply noise generation to the float height data
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            float& Normalized = FloatHeightData[Y * Width + X];

            if (Config->bEnableHeightmapGeneration)
            {
                float Noise = CalculateNoise((float)X, (float)Y, Config, Seed);
                float Noise01 = (Noise + 1.0f) * 0.5f;
                Noise01 = FMath::Pow(Noise01, 1.5f);
                Noise01 *= UseHeightMultiplier;
                Noise01 = FMath::Clamp(Noise01, 0.0f, 1.0f);

                // Blend noise with existing height (from heightmaps)
                if (Config->bEnableImportedHeightmaps && Config->HeightmapLayers.Num() > 0)
                {
                    // For backward compatibility, use additive blending with noise
                    Normalized += Noise01;
                }
                else
                {
                    Normalized = Noise01;
                }

                Normalized = FMath::Clamp(Normalized, 0.0f, 1.0f);
            }
        }
    }

    // Convert float height data to uint16
    for (int32 i = 0; i < FloatHeightData.Num(); i++)
    {
        HeightData[i] = (uint16)(FloatHeightData[i] * 65535.0f);
    }
    if (Config->SmoothingIterations > 0) SmoothHeightData(HeightData, Width, Height, Config->SmoothingIterations);
    
    // Plan first to determine stamp locations
    TArray<FPlannedObject> PlannedObjects;
    TArray<FPlannedStamp> PlannedStamps;
    TMap<UTexture2D*, TArray<float>> TextureCache;
    PlanObjectsAndStamps(this, Config, Seed, Width, Height, HeightData, PlannedObjects, PlannedStamps, TextureCache);

	if (Config->bEnableStamps)
	{
        // Apply baked stamps to local height data for accurate object placement calculations
		for (const FPlannedStamp& Stamp : PlannedStamps)
		{
            if (!Stamp.ActorClass && Stamp.StampTexture)
            {
                TArray<float> Pixels;
                if (const TArray<float>* Cached = TextureCache.Find(Stamp.StampTexture))
                {
                    Pixels = *Cached;
                }
                else
                {
                    int32 TW, TH;
                    if (GetStampData(Stamp.StampTexture, Pixels, TW, TH))
                    {
                        TextureCache.Add(Stamp.StampTexture, Pixels);
                    }
                }
                
                if (Pixels.Num() > 0)
                {
                    ApplyPlannedStampToHeightData(Stamp, HeightData, Width, Height, Pixels, Stamp.TexW, Stamp.TexH);
                }
            }
		}
	}

    // Find Stamp Actors to ignore in raycast
    TArray<AActor*> SpawnedStampActors;
    for (TActorIterator<ALandscapeStampBrush> It(World); It; ++It)
    {
        SpawnedStampActors.Add(*It);
    }

    // Find Water Plane to ignore in raycast
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        if (Actor && Actor->GetFolderPath() == FName("Water"))
        {
            SpawnedStampActors.Add(Actor);
        }
    }

    // Spawn Loop
    int32 ObjCount = 0;
    int32 SlopeSkippedCount = 0;
    
    // Group instances
    TMap<UStaticMesh*, TArray<FTransform>> InstancesToSpawn;
    AActor* InstanceContainer = nullptr;

    for (const FPlannedObject& Obj : PlannedObjects)
    {
        FVector Loc = Obj.Transform.GetLocation();
        
        float LandZScale = ZScale;  
        // Formula: (Normalized - 0.5) * 512 * Scale
        float CalculatedZ = (Obj.ExpectedZ - 0.5f) * 512.0f * LandZScale; 
        CalculatedZ += Landscape->GetActorLocation().Z;
        
        FHitResult Hit;
        FVector Start(Loc.X, Loc.Y, 100000.0f);
        FVector End(Loc.X, Loc.Y, -100000.0f);
        
        FCollisionQueryParams Params;
        Params.AddIgnoredActors(SpawnedStampActors); 
        
        bool bHit = World->LineTraceSingleByObjectType(Hit, Start, End, FCollisionObjectQueryParams(ECC_WorldStatic), Params);
        
        FVector SurfaceNormal = FVector::UpVector;

        if (bHit)
        {
            Loc.Z = Hit.ImpactPoint.Z + Obj.ZOffset;
            SurfaceNormal = Hit.ImpactNormal;
        }
        else 
        {
            Loc.Z = CalculatedZ + Obj.ZOffset;
            SurfaceNormal = FVector::UpVector;
        }

        // --- Slope Constraints Check ---
        if (Obj.bUseSlopeConstraints)
        {
            // Calculate slope angle in degrees
            // Dot product of Normal and UpVector gives Cos(Angle)
            float DotP = FVector::DotProduct(SurfaceNormal, FVector::UpVector);
            float AngleRad = FMath::Acos(DotP);
            float AngleDeg = FMath::RadiansToDegrees(AngleRad);

            if (AngleDeg < Obj.MinSlope || AngleDeg > Obj.MaxSlope)
            {
                // Slope is out of range, skip placement
                SlopeSkippedCount++;
                continue;
            }
        }

        // --- Alignment Logic ---
        FRotator FinalRotation = Obj.Transform.GetRotation().Rotator();

        if (Obj.bAlignToSurface)
        {
            // Start with rotation matching surface normal
            FRotator SurfaceRot = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();
            
            // Apply the random yaw offset
            // We want to rotate around the local Z (which is SurfaceNormal)
            FQuat SurfaceQuat = SurfaceRot.Quaternion();
            FQuat YawQuat = FQuat(FVector::UpVector, FMath::DegreesToRadians(FinalRotation.Yaw)); // Rotate around local up
            
            FQuat FinalQuat = SurfaceQuat * YawQuat; 
            FinalRotation = FinalQuat.Rotator();
        }

        FTransform FinalTransform = Obj.Transform;
        FinalTransform.SetLocation(Loc);
        FinalTransform.SetRotation(FinalRotation.Quaternion());

        // Instancing Check
        if (Obj.bUseInstancing && Obj.StaticMesh)
        {
            InstancesToSpawn.FindOrAdd(Obj.StaticMesh).Add(FinalTransform);
            ObjCount++;
            continue;
        }

        FActorSpawnParameters P;
        P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* NewObj = nullptr;

        if (Obj.ActorClass)
        {
            NewObj = World->SpawnActor<AActor>(Obj.ActorClass, Loc, Obj.Transform.GetRotation().Rotator(), P);
        }
        else if (Obj.StaticMesh)
        {
            AStaticMeshActor* SMA = World->SpawnActor<AStaticMeshActor>(Loc, Obj.Transform.GetRotation().Rotator(), P);
            if (SMA)
            {
                SMA->GetStaticMeshComponent()->SetStaticMesh(Obj.StaticMesh);
                SMA->SetMobility(EComponentMobility::Movable);
                NewObj = SMA;
            }
        }

        if (NewObj)
        {
            NewObj->SetActorScale3D(Obj.Transform.GetScale3D());
#if WITH_EDITOR
            NewObj->SetFolderPath(FName("Actors"));
#endif
            ObjCount++;
        }
    }

    // Process Instances
    if (InstancesToSpawn.Num() > 0)
    {
        // Spawn Container
        FActorSpawnParameters P;
        P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        InstanceContainer = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
        
        if (InstanceContainer)
        {
            InstanceContainer->SetActorLabel(TEXT("Generated_Instances"));
            InstanceContainer->SetFolderPath(FName("Actors"));
            
            // Root component
            USceneComponent* Root = NewObject<USceneComponent>(InstanceContainer, TEXT("Root"));
            InstanceContainer->SetRootComponent(Root);
            Root->SetMobility(EComponentMobility::Static); // Fix: Ensure root is static for HISM attachment
            Root->RegisterComponent();

            for (auto& Elem : InstancesToSpawn)
            {
                UStaticMesh* Mesh = Elem.Key;
                TArray<FTransform>& Transforms = Elem.Value;
                
                UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(InstanceContainer);
                HISM->SetStaticMesh(Mesh);
                // Set mobility to Static for better performance if they don't move, or Movable if they might. 
                // Usually generated terrain props are static.
                HISM->SetMobility(EComponentMobility::Static); 
                HISM->RegisterComponent();
                HISM->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
                
                // Add Instances
                HISM->AddInstances(Transforms, false);

                // FIX: Ensure component is serialized with the actor
                InstanceContainer->AddInstanceComponent(HISM);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("LandGen: Placed %d Assets. (Skipped by Slope: %d)"), ObjCount, SlopeSkippedCount);
}

void ARevoltLandGen::ClearAssets()
{
	UWorld* World = GetWorld();
	if (!World) return;

    // Clear Generated Actors in "Actors" folder
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->GetFolderPath() == FName("Actors"))
        {
            World->EditorDestroyActor(Actor, true);
        }
    }
}

void ARevoltLandGen::ClearLandscape()
{
	UWorld* World = GetWorld();
	if (!World) return;

    ClearAssets(); // Also clear assets

	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		ALandscape* Landscape = *It;
		if (Landscape)
		{
			World->EditorDestroyActor(Landscape, true);
		}
	}

    // Clear Stamps
    for (TActorIterator<ALandscapeStampBrush> It(World); It; ++It)
    {
        World->EditorDestroyActor(*It, true);
    }

    // Clear Water
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        if (Actor && Actor->GetFolderPath() == FName("Water"))
        {
            World->EditorDestroyActor(Actor, true);
        }
    }
}
#endif
