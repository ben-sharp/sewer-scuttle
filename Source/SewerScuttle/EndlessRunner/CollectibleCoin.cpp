// Copyright Epic Games, Inc. All Rights Reserved.

#include "CollectibleCoin.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "RabbitCharacter.h"
#include "EndlessRunnerGameMode.h"

ACollectibleCoin::ACollectibleCoin()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(50.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create particle effect component
	CollectionEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CollectionEffect"));
	CollectionEffect->SetupAttachment(RootComponent);
	CollectionEffect->bAutoActivate = false;
}

void ACollectibleCoin::BeginPlay()
{
	Super::BeginPlay();

	// Set up overlap event
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACollectibleCoin::OnOverlapBegin);

	// Store initial Z position
	InitialZ = GetActorLocation().Z;
}

void ACollectibleCoin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected)
	{
		return;
	}

	// Rotate coin
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);

	// Bob up and down
	float NewZ = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) * BobAmplitude;
	FVector Location = GetActorLocation();
	Location.Z = NewZ;
	SetActorLocation(Location);
}

void ACollectibleCoin::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Early exit if already collected (prevents multiple overlap calls)
	// Check this FIRST before any other processing
	if (bCollected)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("CollectibleCoin: Ignoring overlap - already collected (Actor: %s)"), *GetName());
		return;
	}

	ARabbitCharacter* Player = Cast<ARabbitCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// Only respond to the player's capsule component to prevent multiple overlap events
	// The character's capsule is the main collision component
	if (OtherComp != Player->GetCapsuleComponent())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("CollectibleCoin: Ignoring overlap - not capsule component (Component: %s, Actor: %s)"), *OtherComp->GetName(), *GetName());
		return;
	}

	// Disable collision IMMEDIATELY to prevent multiple overlap events
	// This must happen before Collect() to prevent race conditions
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Also remove the overlap delegate to be extra safe
		CollisionSphere->OnComponentBeginOverlap.RemoveAll(this);
	}
	
	UE_LOG(LogTemp, Log, TEXT("CollectibleCoin: Collecting coin (Value: %d, Actor: %s)"), Value, *GetName());
	Collect();
}

void ACollectibleCoin::Collect()
{
	// Early exit if already collected (prevents double collection)
	// Note: bCollected should already be set in OnOverlapBegin, but check again for safety
	// This protects against Blueprint calls or other code paths
	if (bCollected)
	{
		UE_LOG(LogTemp, Warning, TEXT("CollectibleCoin: Collect() called but already collected! (Actor: %s) - This should not happen if called from OnOverlapBegin"), *GetName());
		return;
	}

	// Set collected flag (should already be set in OnOverlapBegin, but ensure it for Blueprint calls)
	bCollected = true;

	// Play collection effect
	if (CollectionEffect)
	{
		CollectionEffect->Activate();
	}

	// Hide mesh
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false);
	}

	// Disable collision
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Calculate final value (apply special multiplier if special collectible)
	int32 FinalValue = Value;
	if (bIsSpecial)
	{
		FinalValue = FMath::FloorToInt(Value * SpecialValueMultiplier);
		UE_LOG(LogTemp, Log, TEXT("CollectibleCoin: Special collectible - Value %d * %.2f = %d"), Value, SpecialValueMultiplier, FinalValue);
	}

	// Add to run currency (temporary, resets each game)
	if (UWorld* World = GetWorld())
	{
		if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
		{
			GameMode->AddRunCurrency(FinalValue);
		}
	}

	// Destroy after effect
	SetLifeSpan(1.0f);
}

