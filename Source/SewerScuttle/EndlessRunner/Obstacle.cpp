// Copyright Epic Games, Inc. All Rights Reserved.

#include "Obstacle.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RabbitCharacter.h"
#include "RabbitJumpComponent.h"
#include "RabbitSlideComponent.h"
#include "EndlessRunnerGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

AObstacle::AObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision box
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AObstacle::BeginPlay()
{
	Super::BeginPlay();

	// Set up overlap event
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AObstacle::OnOverlapBegin);

	// Only adjust collision box based on obstacle type if it's still at default values
	// This allows Blueprint customization to persist
	FVector CurrentExtent = CollisionBox->GetUnscaledBoxExtent();
	FVector DefaultExtent = FVector(50.0f, 50.0f, 50.0f);
	
	// Check if extents are still at default (within small tolerance)
	if (FVector::Dist(CurrentExtent, DefaultExtent) < 1.0f)
	{
		// Adjust collision box based on obstacle type (only if not customized)
		switch (ObstacleType)
		{
		case EObstacleType::Low:
			// Low obstacle - can slide under
			CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 30.0f));
			CollisionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
			break;
		case EObstacleType::High:
			// High obstacle - can jump over
			CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
			CollisionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
			break;
		case EObstacleType::Full:
			// Full obstacle - must avoid
			CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
			CollisionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
			break;
		}
	}
	// If extents have been customized in Blueprint, don't override them
}

void AObstacle::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(OtherActor);
	if (Player)
	{
		// Check if Enforcer can break this obstacle FIRST (before any other checks)
		// This allows Enforcer to break obstacles regardless of avoidance state
		if (bIsBreakable && Player->GetCanBreakObstacles())
		{
			// Enforcer breaks through breakable obstacle
			UE_LOG(LogTemp, Warning, TEXT("Obstacle: Enforcer breaking through breakable obstacle '%s'"), *GetName());
			
			// Play break effect if available
			if (BreakEffect && GetWorld())
			{
				FVector EffectLocation = GetActorLocation();
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakEffect, EffectLocation);
			}
			
			// Destroy obstacle (no damage to player)
			Destroy();
			return;
		}

		// Check if player can avoid this obstacle
		bool bCanAvoid = false;

		if (ObstacleType == EObstacleType::Low)
		{
			// Check if player is sliding OR if Scout class (never needs crouch)
			if (Player->GetSlideComponent() && Player->GetSlideComponent()->IsSliding())
			{
				bCanAvoid = true;
			}
			else if (Player->GetNeverNeedsCrouch())
			{
				// Scout class can pass under low obstacles without sliding
				bCanAvoid = true;
				UE_LOG(LogTemp, Log, TEXT("Obstacle: Scout class passing under low obstacle without sliding"));
			}
		}
		else if (ObstacleType == EObstacleType::High)
		{
			// Check if player is jumping
			if (Player->GetJumpComponent() && Player->GetJumpComponent()->IsJumping())
			{
				bCanAvoid = true;
			}
		}

		if (!bCanAvoid)
		{
			// Check if player is invincible
			if (Player->IsInvincible())
			{
				UE_LOG(LogTemp, Log, TEXT("Obstacle: Player is invincible, ignoring collision from obstacle '%s'"), *GetName());
				return;
			}
			
			// Check if this obstacle should force instant death
			bool bShouldInstantKill = false;
			if (bIgnoreLastStand)
			{
				// Get current lives from GameMode
				if (UWorld* World = GetWorld())
				{
					if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
					{
						int32 CurrentLives = GameMode->GetLives();
						if (LivesLost >= CurrentLives)
						{
							bShouldInstantKill = true;
							UE_LOG(LogTemp, Warning, TEXT("Obstacle: Instant death obstacle '%s' - LivesLost (%d) >= CurrentLives (%d)"), 
								*GetName(), LivesLost, CurrentLives);
						}
					}
				}
			}
			
			// Notify GameMode about obstacle hit
			UE_LOG(LogTemp, Warning, TEXT("Obstacle: Player hit obstacle '%s' (Type=%d, LivesLost=%d, InstantDeath=%d)"), 
				*GetName(), (int32)ObstacleType, LivesLost, bShouldInstantKill ? 1 : 0);
			if (UWorld* World = GetWorld())
			{
				if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
				{
					GameMode->OnPlayerHitObstacle(LivesLost, bShouldInstantKill);
				}
			}

			// Apply damage and knockback (Blueprint event)
			OnPlayerCollision(Player);

			// Apply knockback
			UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement();
			if (MovementComp)
			{
				FVector KnockbackDirection = -Player->GetActorForwardVector();
				MovementComp->AddImpulse(KnockbackDirection * KnockbackForce);
			}
		}
	}
}

