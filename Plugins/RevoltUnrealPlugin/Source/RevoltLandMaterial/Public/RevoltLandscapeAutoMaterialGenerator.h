#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Materials/Material.h"
#include "RevoltLandscapeAutoMaterialConfig.h"
#include "RevoltLandscapeAutoMaterialGenerator.generated.h"

/**
 * Generates a fixed-topology landscape auto-material based on the 4-layer config.
 */
UCLASS()
class REVOLTLANDMATERIAL_API URevoltLandscapeAutoMaterialGenerator : public UObject
{
	GENERATED_BODY()

public:
	static UMaterial* GenerateAutoMaterial(URevoltLandscapeAutoMaterialConfig* Config);
};

