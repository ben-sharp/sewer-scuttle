#include "RevoltLandGenConfig.h"
#include "Engine/AssetManager.h"

URevoltLandGenConfig::URevoltLandGenConfig()
{
	// Default constructor - no preset metadata needed
}

void URevoltLandGenConfig::ValidateConfig()
{
	// Clamp values to valid ranges
	NoiseScale = FMath::Clamp(NoiseScale, 0.001f, 10.0f);
	HeightMultiplier = FMath::Clamp(HeightMultiplier, 0.1f, 50.0f);
	Octaves = FMath::Clamp(Octaves, 1, 8);
	Persistence = FMath::Clamp(Persistence, 0.1f, 1.0f);
	Lacunarity = FMath::Clamp(Lacunarity, 1.0f, 4.0f);
    SmoothingIterations = FMath::Clamp(SmoothingIterations, 0, 10);

	UE_LOG(LogTemp, Log, TEXT("Configuration validated successfully"));
}

#if WITH_EDITOR
void URevoltLandGenConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-validate when properties change in editor
	if (PropertyChangedEvent.Property)
	{
		ValidateConfig();
	}
}
#endif

FPrimaryAssetId URevoltLandGenConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("RevoltLandGenConfig", GetFName());
}
