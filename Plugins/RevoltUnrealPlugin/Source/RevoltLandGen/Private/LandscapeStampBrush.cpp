#include "LandscapeStampBrush.h"
#include "Components/BoxComponent.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Landscape.h"
#include "EngineUtils.h"

ALandscapeStampBrush::ALandscapeStampBrush()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create a root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create a wireframe box to visualize the stamp bounds
	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Box->SetupAttachment(Root);
	Box->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f)); // Default 1m box
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetVisibility(true);
	Box->SetHiddenInGame(true); // Only visible in editor
    
    // Make the box lines visible? BoxComponent draws wireframe when selected or in editor.
    Box->ShapeColor = FColor::Cyan;

	SetCanAffectHeightmap(true);
    SetActorHiddenInGame(false); 
}

UTextureRenderTarget2D* ALandscapeStampBrush::DrawStampToRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
#if WITH_EDITOR
    UE_LOG(LogTemp, Log, TEXT("StampBrush: DrawStampToRenderTarget Called. Texture: %s"), StampTexture ? *StampTexture->GetName() : TEXT("None"));

    if (!InRenderTarget || !StampTexture) return InRenderTarget;
    
    UWorld* World = GetWorld();
    if (!World) return InRenderTarget;
    
    ALandscape* Landscape = GetOwningLandscape();
    if (!Landscape) 
    {
        // Try to find landscape if owning is not set (e.g. if we haven't been fully initialized by the layer system yet)
         for (TActorIterator<ALandscape> It(World); It; ++It)
         {
             Landscape = *It;
             break;
         }
    }
    
    if (!Landscape) return InRenderTarget;

    FTransform LandscapeTransform = Landscape->GetActorTransform();
    FVector LandscapeScale = LandscapeTransform.GetScale3D();
    FVector LandscapeLocation = LandscapeTransform.GetLocation(); 

    FVector BrushLocation = GetActorLocation();
    FVector BrushScale = GetActorScale3D(); 
    
    float BaseSize = 100.0f;
    float BrushWorldSizeX = BaseSize * BrushScale.X; 
    float BrushWorldSizeY = BaseSize * BrushScale.Y;
    
    float LScaleX = LandscapeScale.X; 
    float LScaleY = LandscapeScale.Y;
    
    // Relative World Position
    FVector RelPos = BrushLocation - LandscapeLocation;
    
    float PixelX = RelPos.X / LScaleX;
    float PixelY = RelPos.Y / LScaleY;
    
    float PixelSizeX = BrushWorldSizeX / LScaleX;
    float PixelSizeY = BrushWorldSizeY / LScaleY;
    
    // Center alignment
    float DrawX = PixelX - (PixelSizeX * 0.5f);
    float DrawY = PixelY - (PixelSizeY * 0.5f);
    
    FRotator BrushRotation = GetActorRotation();

    FCanvas Canvas(InRenderTarget->GameThread_GetRenderTargetResource(), nullptr, World, World->GetFeatureLevel());
    
    // Use FCanvasTileItem with FTexture* resource
    FCanvasTileItem TileItem(FVector2D(DrawX, DrawY), StampTexture->GetResource(), FVector2D(PixelSizeX, PixelSizeY), FLinearColor(Intensity, Intensity, Intensity, 1.0f));
    TileItem.BlendMode = SE_BLEND_Additive; 
    
    Canvas.DrawItem(TileItem);
    Canvas.Flush_GameThread();
#endif

    return InRenderTarget;
}

void ALandscapeStampBrush::BeginPlay()
{
	Super::BeginPlay();
}

