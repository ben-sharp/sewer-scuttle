// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUp.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "RabbitCharacter.h"
#include "EndlessRunnerGameMode.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "RabbitAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"

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
	// Early exit if already collected
	if (bCollected)
	{
		return;
	}

	ARabbitCharacter* Player = Cast<ARabbitCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// Only respond to the player's capsule component to prevent multiple overlap events
	if (OtherComp != Player->GetCapsuleComponent())
	{
		return;
	}

	// Disable collision immediately to prevent multiple overlap events
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionSphere->OnComponentBeginOverlap.RemoveAll(this);
	}

	Collect(Player);
}

void APowerUp::Collect(ARabbitCharacter* Player)
{
	if (bCollected || !Player)
	{
		return;
	}

	bCollected = true;

	// Apply via GAS (required)
	if (PermanentStatModifierEffect || TemporaryMultiplierEffect)
	{
		ApplyGASPowerUp(Player);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PowerUp: No GAS effects configured! Power-up must have PermanentStatModifierEffect or TemporaryMultiplierEffect set."));
	}

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
	// If no restrictions, available for all classes
	if (AllowedClasses.Num() == 0)
	{
		return true;
	}
	
	return AllowedClasses.Contains(PlayerClass);
}

void APowerUp::ApplyGASPowerUp(ARabbitCharacter* Player)
{
	if (!Player || !Player->GetAbilitySystemComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("PowerUp: Cannot apply GAS powerup - Player or AbilitySystemComponent is null"));
		return;
	}

	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	URabbitAttributeSet* AttributeSet = Player->GetAttributeSet();
	
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("PowerUp: Cannot apply GAS powerup - AttributeSet is null"));
		return;
	}

	// Helper function to get attribute from stat type name
	auto GetAttributeFromStatType = [AttributeSet](const FName& StatType, bool bIsPermanent) -> FGameplayAttribute
	{
		// Map stat type names to attributes
		// For permanent modifications, modify base attributes
		// For temporary multipliers, modify multiplier attributes
		if (bIsPermanent)
		{
			if (StatType == "Speed" || StatType == "BaseSpeed")
				return AttributeSet->GetBaseSpeedAttribute();
			else if (StatType == "JumpHeight" || StatType == "BaseJumpHeight")
				return AttributeSet->GetBaseJumpHeightAttribute();
			else if (StatType == "Lives" || StatType == "BaseLives")
				return AttributeSet->GetBaseLivesAttribute();
			else if (StatType == "MaxJumpCount" || StatType == "BaseMaxJumpCount" || StatType == "CurrentMaxJumpCount")
				return AttributeSet->GetBaseMaxJumpCountAttribute(); // Always modify base for permanent changes
			else if (StatType == "LaneTransitionSpeed" || StatType == "BaseLaneTransitionSpeed")
				return AttributeSet->GetBaseLaneTransitionSpeedAttribute();
		}
		else
		{
			if (StatType == "Speed" || StatType == "SpeedMultiplier")
				return AttributeSet->GetSpeedMultiplierAttribute();
			else if (StatType == "JumpHeight" || StatType == "JumpHeightMultiplier")
				return AttributeSet->GetJumpHeightMultiplierAttribute();
			else if (StatType == "CoinMultiplier")
				return AttributeSet->GetCoinMultiplierAttribute();
			else if (StatType == "ScoreMultiplier")
				return AttributeSet->GetScoreMultiplierAttribute();
			else if (StatType == "LaneTransitionSpeed" || StatType == "LaneTransitionSpeedMultiplier")
				return AttributeSet->GetLaneTransitionSpeedMultiplierAttribute();
			else if (StatType == "Magnet" || StatType == "MagnetActive")
				return AttributeSet->GetMagnetActiveAttribute();
			else if (StatType == "Autopilot" || StatType == "AutopilotActive")
				return AttributeSet->GetAutopilotActiveAttribute();
			else if (StatType == "Invincibility" || StatType == "InvincibilityActive")
				return AttributeSet->GetInvincibilityActiveAttribute();
		}
		
		return FGameplayAttribute(); // Invalid attribute
	};

	// Apply permanent base stat modification if GameplayEffect is set
	if (PermanentStatModifierEffect)
	{
		FGameplayAttribute TargetAttribute = GetAttributeFromStatType(StatTypeToModify, true);
		if (TargetAttribute.IsValid() && AttributeSet)
		{
			// For permanent base stat modifications, manually accumulate the value
			// GAS doesn't stack infinite duration effects properly, so we read current value, add to it, and set directly
			float CurrentBaseValue = 0.0f;
			
			// Get current base value based on stat type
			if (StatTypeToModify == "Speed" || StatTypeToModify == "BaseSpeed")
				CurrentBaseValue = AttributeSet->GetBaseSpeed();
			else if (StatTypeToModify == "JumpHeight" || StatTypeToModify == "BaseJumpHeight")
				CurrentBaseValue = AttributeSet->GetBaseJumpHeight();
			else if (StatTypeToModify == "Lives" || StatTypeToModify == "BaseLives")
				CurrentBaseValue = AttributeSet->GetBaseLives();
			else if (StatTypeToModify == "MaxJumpCount" || StatTypeToModify == "BaseMaxJumpCount" || StatTypeToModify == "CurrentMaxJumpCount")
				CurrentBaseValue = AttributeSet->GetBaseMaxJumpCount();
			else if (StatTypeToModify == "LaneTransitionSpeed" || StatTypeToModify == "BaseLaneTransitionSpeed")
				CurrentBaseValue = AttributeSet->GetBaseLaneTransitionSpeed();
			
			// Add the modification value
			float NewBaseValue = CurrentBaseValue + ModificationValue;
			
			UE_LOG(LogTemp, Warning, TEXT("PowerUp: Applying permanent stat modifier - StatType: %s, Current: %.2f, Adding: %.2f, New: %.2f"), 
				*StatTypeToModify.ToString(), CurrentBaseValue, ModificationValue, NewBaseValue);
			
			// Set the new base value directly
			if (StatTypeToModify == "Speed" || StatTypeToModify == "BaseSpeed")
				AttributeSet->SetBaseSpeed(NewBaseValue);
			else if (StatTypeToModify == "JumpHeight" || StatTypeToModify == "BaseJumpHeight")
				AttributeSet->SetBaseJumpHeight(NewBaseValue);
			else if (StatTypeToModify == "Lives" || StatTypeToModify == "BaseLives")
				AttributeSet->SetBaseLives(NewBaseValue);
			else if (StatTypeToModify == "MaxJumpCount" || StatTypeToModify == "BaseMaxJumpCount" || StatTypeToModify == "CurrentMaxJumpCount")
				AttributeSet->SetBaseMaxJumpCount(NewBaseValue);
			else if (StatTypeToModify == "LaneTransitionSpeed" || StatTypeToModify == "BaseLaneTransitionSpeed")
				AttributeSet->SetBaseLaneTransitionSpeed(NewBaseValue);
			
			UE_LOG(LogTemp, Log, TEXT("PowerUp: Successfully applied permanent stat modifier (Stat: %s, New Base Value: %.2f)"), 
				*StatTypeToModify.ToString(), NewBaseValue);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PowerUp: Invalid stat type for permanent modification: %s"), *StatTypeToModify.ToString());
		}
	}

	// Apply temporary multiplier if GameplayEffect is set
	if (TemporaryMultiplierEffect && Duration > 0.0f)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TemporaryMultiplierEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set duration (second parameter locks duration)
			SpecHandle.Data->SetDuration(Duration, false);
			
			FGameplayAttribute TargetAttribute = GetAttributeFromStatType(StatTypeToModify, false);
			if (TargetAttribute.IsValid())
			{
				FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
				if (Spec)
				{
					// Use SetByCaller to set the magnitude
					// The GameplayEffect must be configured in Blueprint to use SetByCaller magnitude for this attribute
					// Use StatTypeToModify as the tag name (must match the SetByCaller tag in the GameplayEffect Blueprint)
					Spec->SetSetByCallerMagnitude(StatTypeToModify, ModificationValue);
					ASC->ApplyGameplayEffectSpecToSelf(*Spec);
					UE_LOG(LogTemp, Log, TEXT("PowerUp: Applied temporary multiplier via GAS (Stat: %s, Value: %.2f, Duration: %.2f)"), 
						*StatTypeToModify.ToString(), ModificationValue, Duration);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PowerUp: Invalid stat type for temporary multiplier: %s"), *StatTypeToModify.ToString());
			}
		}
	}
}

