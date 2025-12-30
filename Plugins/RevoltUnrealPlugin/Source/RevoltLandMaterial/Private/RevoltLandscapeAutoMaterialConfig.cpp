#include "RevoltLandscapeAutoMaterialConfig.h"
#include "RevoltLandscapeAutoMaterialGenerator.h"

void URevoltLandscapeAutoMaterialConfig::GenerateMaterial()
{
	URevoltLandscapeAutoMaterialGenerator::GenerateAutoMaterial(this);
}

