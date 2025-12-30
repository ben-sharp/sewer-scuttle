#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrushBase.h"
#include "LandscapeStampBrush.generated.h"

UCLASS(Blueprintable)
class REVOLTLANDGEN_API ALandscapeStampBrush : public ALandscapeBlueprintBrushBase
{
	GENERATED_BODY()
	
public:	
	ALandscapeStampBrush();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	UTexture2D* StampTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamp")
	float Intensity = 1.0f;

	/** Call this from the Blueprint Render event to draw the stamp */
	UFUNCTION(BlueprintCallable, Category = "Stamp")
	UTextureRenderTarget2D* DrawStampToRenderTarget(UTextureRenderTarget2D* InRenderTarget);

protected:
	virtual void BeginPlay() override;

};

