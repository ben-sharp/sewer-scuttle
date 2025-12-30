#include "RevoltLandscapeAutoMaterialGenerator.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionLandscapeLayerCoords.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionVertexNormalWS.h" // Use VertexNormal for slope to avoid PixelNormalWS errors
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "UObject/SavePackage.h"
#include "Editor.h"
#endif

UMaterial* URevoltLandscapeAutoMaterialGenerator::GenerateAutoMaterial(URevoltLandscapeAutoMaterialConfig* Config)
{
#if WITH_EDITOR
	if (!Config) return nullptr;

	FString MaterialName = Config->MaterialName;
	FString SavePath = Config->SavePath;

	// Create package
	FString PackageName = SavePath + "/" + MaterialName;
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	// Create Material
	auto MaterialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = (UMaterial*)MaterialFactory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, *MaterialName, RF_Public | RF_Standalone, nullptr, GWarn);

	Material->SetShadingModel(MSM_DefaultLit);

	// --- 1. GLOBAL COORDINATES ---
	UMaterialExpressionLandscapeLayerCoords* Coords = NewObject<UMaterialExpressionLandscapeLayerCoords>(Material);
	Material->GetExpressionCollection().AddExpression(Coords);
	Coords->MaterialExpressionEditorX = -2000;
	Coords->MaterialExpressionEditorY = 0;
	Coords->MappingType = TCMT_Auto;

	// --- 2. GLOBAL HEIGHT & SLOPE MASKS ---
	
	// Height = WorldPos.Z
	UMaterialExpressionWorldPosition* WorldPos = NewObject<UMaterialExpressionWorldPosition>(Material);
	Material->GetExpressionCollection().AddExpression(WorldPos);
	WorldPos->MaterialExpressionEditorX = -2000;
	WorldPos->MaterialExpressionEditorY = -400;

	UMaterialExpressionComponentMask* HeightMask = NewObject<UMaterialExpressionComponentMask>(Material);
	Material->GetExpressionCollection().AddExpression(HeightMask);
	HeightMask->MaterialExpressionEditorX = -1800;
	HeightMask->MaterialExpressionEditorY = -400;
	HeightMask->Input.Expression = WorldPos;
	HeightMask->R = HeightMask->G = HeightMask->A = false;
	HeightMask->B = true; // Z

	// Slope = Dot(VertexNormalWS, UpVector)
	// Using VertexNormalWS is safer for Landscape than PixelNormalWS (which causes SM6 errors in VS/PS dependency)
	UMaterialExpressionVertexNormalWS* VertexNormal = NewObject<UMaterialExpressionVertexNormalWS>(Material);
	Material->GetExpressionCollection().AddExpression(VertexNormal);
	VertexNormal->MaterialExpressionEditorX = -2000;
	VertexNormal->MaterialExpressionEditorY = 400;

	UMaterialExpressionConstant3Vector* UpVector = NewObject<UMaterialExpressionConstant3Vector>(Material);
	Material->GetExpressionCollection().AddExpression(UpVector);
	UpVector->MaterialExpressionEditorX = -2000;
	UpVector->MaterialExpressionEditorY = 600;
	UpVector->Constant = FLinearColor(0, 0, 1, 1);

	UMaterialExpressionDotProduct* SlopeDot = NewObject<UMaterialExpressionDotProduct>(Material);
	Material->GetExpressionCollection().AddExpression(SlopeDot);
	SlopeDot->MaterialExpressionEditorX = -1800;
	SlopeDot->MaterialExpressionEditorY = 500;
	SlopeDot->A.Expression = VertexNormal;
	SlopeDot->B.Expression = UpVector;

	// --- 3. HELPER: Create Mask Function ---
	auto CreateMask = [&](UMaterialExpression* Input, float Threshold, float Falloff, bool bInvert) -> UMaterialExpression* {
		// (Input - Threshold)
		UMaterialExpressionSubtract* Sub = NewObject<UMaterialExpressionSubtract>(Material);
		Material->GetExpressionCollection().AddExpression(Sub);
		Sub->A.Expression = Input;
		
		UMaterialExpressionConstant* ThreshNode = NewObject<UMaterialExpressionConstant>(Material);
		Material->GetExpressionCollection().AddExpression(ThreshNode);
		ThreshNode->R = Threshold;
		Sub->B.Expression = ThreshNode;

		// Divide by Falloff
		UMaterialExpressionDivide* Div = NewObject<UMaterialExpressionDivide>(Material);
		Material->GetExpressionCollection().AddExpression(Div);
		Div->A.Expression = Sub;

		UMaterialExpressionConstant* FalloffNode = NewObject<UMaterialExpressionConstant>(Material);
		Material->GetExpressionCollection().AddExpression(FalloffNode);
		FalloffNode->R = FMath::Max(Falloff, 0.001f); // Avoid divide by zero
		Div->B.Expression = FalloffNode;

		// Clamp 0-1
		UMaterialExpressionClamp* Clamp = NewObject<UMaterialExpressionClamp>(Material);
		Material->GetExpressionCollection().AddExpression(Clamp);
		Clamp->Input.Expression = Div;
		Clamp->MinDefault = 0.0f;
		Clamp->MaxDefault = 1.0f;

		if (bInvert)
		{
			// Used for Slope where Lower Dot = Steeper.
			// We want Mask = 1 when Dot < Threshold (Steep).
			// Current: (Dot - Threshold)/Falloff.
			// Example: Thresh 0.7. Dot 0.5 (Steep). (0.5 - 0.7) = -0.2. Negative.
			// Example: Dot 0.9 (Flat). (0.9 - 0.7) = 0.2. Positive.
			
			// Actually for slope (Steepness):
			// We want 1 when Steep (Low Dot).
			// OneMinus((Dot - (Threshold - Falloff)) / Falloff)
			// Let's do a simple manual inversion logic:
			// If Dot < Threshold, we want Alpha 1.
			// Let's use: Clamp( (Threshold - Dot) / Falloff )
			// If Thresh 0.7, Dot 0.5 -> (0.2)/Falloff -> Positive -> Clamp 1.
			// If Thresh 0.7, Dot 0.9 -> (-0.2)/Falloff -> Negative -> Clamp 0.
			
			UMaterialExpressionSubtract* InvSub = NewObject<UMaterialExpressionSubtract>(Material);
			Material->GetExpressionCollection().AddExpression(InvSub);
			
			UMaterialExpressionConstant* InvThresh = NewObject<UMaterialExpressionConstant>(Material);
			Material->GetExpressionCollection().AddExpression(InvThresh);
			InvThresh->R = Threshold;
			InvSub->A.Expression = InvThresh; // Threshold first
			InvSub->B.Expression = Input; // Minus Input

			UMaterialExpressionDivide* InvDiv = NewObject<UMaterialExpressionDivide>(Material);
			Material->GetExpressionCollection().AddExpression(InvDiv);
			InvDiv->A.Expression = InvSub;
			InvDiv->B.Expression = FalloffNode;

			UMaterialExpressionClamp* InvClamp = NewObject<UMaterialExpressionClamp>(Material);
			Material->GetExpressionCollection().AddExpression(InvClamp);
			InvClamp->Input.Expression = InvDiv;
			InvClamp->MinDefault = 0.0f;
			InvClamp->MaxDefault = 1.0f;
			
			return InvClamp;
		}

		return Clamp;
	};

	// --- 4. CALCULATE MASKS ---
	UMaterialExpression* MidMaskNode = CreateMask(HeightMask, Config->MidStartHeight, Config->MidFalloff, false);
	UMaterialExpression* HighMaskNode = CreateMask(HeightMask, Config->HighStartHeight, Config->HighFalloff, false);
	UMaterialExpression* SlopeMaskNode = CreateMask(SlopeDot, Config->SlopeThreshold, Config->SlopeFalloff, true);

	// --- 5. CREATE LAYERS (Helper) ---
	auto CreateLayer = [&](const FRevoltBiomeLayer& LayerData, int32 OffsetY) -> TTuple<UMaterialExpression*, UMaterialExpression*, UMaterialExpression*> {
		// Tiling Logic: Coords * Tiling
		UMaterialExpressionConstant* TilingConst = NewObject<UMaterialExpressionConstant>(Material);
		Material->GetExpressionCollection().AddExpression(TilingConst);
		TilingConst->MaterialExpressionEditorX = -700;
		TilingConst->MaterialExpressionEditorY = OffsetY;
		TilingConst->R = FMath::Max(LayerData.Tiling, 0.0001f); // Protect against 0

		UMaterialExpressionMultiply* TilingMult = NewObject<UMaterialExpressionMultiply>(Material);
		Material->GetExpressionCollection().AddExpression(TilingMult);
		TilingMult->MaterialExpressionEditorX = -600;
		TilingMult->MaterialExpressionEditorY = OffsetY;
		TilingMult->A.Expression = Coords;
		TilingMult->B.Expression = TilingConst;

		// Diffuse
		UMaterialExpressionTextureSample* Diff = NewObject<UMaterialExpressionTextureSample>(Material);
		Material->GetExpressionCollection().AddExpression(Diff);
		Diff->MaterialExpressionEditorX = -500;
		Diff->MaterialExpressionEditorY = OffsetY;
		Diff->Coordinates.Expression = TilingMult;
		Diff->SamplerType = SAMPLERTYPE_Color; // Set BEFORE texture
		if (LayerData.Albedo) Diff->Texture = LayerData.Albedo;
		Diff->SamplerType = SAMPLERTYPE_Color; // Set AFTER texture to force it

		// Normal
		UMaterialExpressionTextureSample* Norm = NewObject<UMaterialExpressionTextureSample>(Material);
		Material->GetExpressionCollection().AddExpression(Norm);
		Norm->MaterialExpressionEditorX = -500;
		Norm->MaterialExpressionEditorY = OffsetY + 200;
		Norm->Coordinates.Expression = TilingMult;
		Norm->SamplerType = SAMPLERTYPE_Normal; // Set BEFORE texture
		if (LayerData.Normal) Norm->Texture = LayerData.Normal;
		Norm->SamplerType = SAMPLERTYPE_Normal; // Set AFTER texture to force it

		// Roughness
		UMaterialExpression* Rough = nullptr;
		if (LayerData.Roughness)
		{
			UMaterialExpressionTextureSample* RoughTex = NewObject<UMaterialExpressionTextureSample>(Material);
			Material->GetExpressionCollection().AddExpression(RoughTex);
			RoughTex->MaterialExpressionEditorX = -500;
			RoughTex->MaterialExpressionEditorY = OffsetY + 400;
			RoughTex->Coordinates.Expression = TilingMult;
			RoughTex->SamplerType = SAMPLERTYPE_LinearColor; // Set BEFORE texture
			RoughTex->Texture = LayerData.Roughness;
			RoughTex->SamplerType = SAMPLERTYPE_LinearColor; // Set AFTER texture
			Rough = RoughTex;
		}
		else
		{
			UMaterialExpressionConstant* RoughConst = NewObject<UMaterialExpressionConstant>(Material);
			Material->GetExpressionCollection().AddExpression(RoughConst);
			RoughConst->MaterialExpressionEditorX = -500;
			RoughConst->MaterialExpressionEditorY = OffsetY + 400;
			RoughConst->R = LayerData.RoughnessVal;
			Rough = RoughConst;
		}

		UE_LOG(LogTemp, Log, TEXT("Layer Generated. Normal Sampler Enum Value: %d (Should be 1/Normal)"), (int32)Norm->SamplerType);

		return MakeTuple((UMaterialExpression*)Diff, (UMaterialExpression*)Norm, Rough);
	};

	auto L_Ground = CreateLayer(Config->GroundLayer, -1000);
	auto L_Mid = CreateLayer(Config->MidLayer, -500);
	auto L_High = CreateLayer(Config->HighLayer, 0);
	auto L_Slope = CreateLayer(Config->SlopeLayer, 500);

	// --- 6. BLEND LOGIC ---
	// 1. Ground + Mid (using MidMask) -> Result1
	// 2. Result1 + High (using HighMask) -> Result2
	// 3. Result2 + Slope (using SlopeMask) -> Final

	auto Blend = [&](UMaterialExpression* A, UMaterialExpression* B, UMaterialExpression* Alpha, int32 X, int32 Y) -> UMaterialExpression* {
		UMaterialExpressionLinearInterpolate* Lerp = NewObject<UMaterialExpressionLinearInterpolate>(Material);
		Material->GetExpressionCollection().AddExpression(Lerp);
		Lerp->MaterialExpressionEditorX = X;
		Lerp->MaterialExpressionEditorY = Y;
		Lerp->A.Expression = A;
		Lerp->B.Expression = B;
		Lerp->Alpha.Expression = Alpha;
		return Lerp;
	};

	// Diffuse Chain
	auto Diff1 = Blend(L_Ground.Get<0>(), L_Mid.Get<0>(), MidMaskNode, 0, -1000);
	auto Diff2 = Blend(Diff1, L_High.Get<0>(), HighMaskNode, 200, -1000);
	auto DiffFinal = Blend(Diff2, L_Slope.Get<0>(), SlopeMaskNode, 400, -1000);

	// Normal Chain
	auto Norm1 = Blend(L_Ground.Get<1>(), L_Mid.Get<1>(), MidMaskNode, 0, -500);
	auto Norm2 = Blend(Norm1, L_High.Get<1>(), HighMaskNode, 200, -500);
	auto NormFinal = Blend(Norm2, L_Slope.Get<1>(), SlopeMaskNode, 400, -500);

	// Roughness Chain
	auto Rough1 = Blend(L_Ground.Get<2>(), L_Mid.Get<2>(), MidMaskNode, 0, 0);
	auto Rough2 = Blend(Rough1, L_High.Get<2>(), HighMaskNode, 200, 0);
	auto RoughFinal = Blend(Rough2, L_Slope.Get<2>(), SlopeMaskNode, 400, 0);

	// --- 7. OUTPUT ---
	Material->GetEditorOnlyData()->BaseColor.Expression = DiffFinal;
	Material->GetEditorOnlyData()->Normal.Expression = NormFinal;
	Material->GetEditorOnlyData()->Roughness.Expression = RoughFinal;

	Material->PostEditChange();
	Material->MarkPackageDirty();

	// Save
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.Error = GError;
	SaveArgs.bForceByteSwapping = true;
	SaveArgs.bWarnOfLongFilename = true;
	SaveArgs.SaveFlags = SAVE_NoError;
	UPackage::SavePackage(Package, Material, *PackageFileName, SaveArgs);

	FAssetRegistryModule::AssetCreated(Material);

	UE_LOG(LogTemp, Log, TEXT("Generated Fixed Auto-Material: %s"), *PackageName);

	return Material;
#else
	return nullptr;
#endif
}

