// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUp.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RabbitCharacter.h"
#include "PlayerClass.h"
#include "GameplayTags.h"
#include "AbilitySystemComponent.h"

APowerUp::APowerUp()
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
}

void APowerUp::BeginPlay()
{
	Super::BeginPlay();

	// Set up overlap event
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APowerUp::OnOverlapBegin);
}

void APowerUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected)
	{
		return;
	}

	// Rotate powerup
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);
}

void APowerUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(OtherActor);
	if (Player && !bCollected)
	{
		Collect(Player);
	}
}

void APowerUp::Collect(ARabbitCharacter* Player)
{
	if (bCollected || !Player)
	{
		return;
	}

	bCollected = true;

	// Call blueprint event
	OnPowerUpCollected(Player);

	// Hide and destroy
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(false);
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetLifeSpan(0.5f);
}

bool APowerUp::IsValidForClass(EPlayerClass PlayerClass) const
{
	// If AllowedClasses is empty, power-up is available for all classes
	if (AllowedClasses.Num() == 0)
	{
		return true;
	}

	// Check if the player class is in the allowed list
	return AllowedClasses.Contains(PlayerClass);
}

void APowerUp::ApplyGASPowerUp(ARabbitCharacter* Player)
{
	if (!Player || !Player->GetAbilitySystemComponent())
	{
		return;
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();

	// Apply permanent stat modifier if set
	if (PermanentStatModifierEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(PermanentStatModifierEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set the modification value using SetByCaller
			if (StatTypeToModify.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(StatTypeToModify, ModificationValue);
			}

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Apply temporary multiplier if set
	if (TemporaryMultiplierEffect && Duration > 0.0f)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TemporaryMultiplierEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set duration
			SpecHandle.Data->SetDuration(Duration, false);

			// Set the modification value using SetByCaller
			if (StatTypeToModify.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(StatTypeToModify, ModificationValue);
			}

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

