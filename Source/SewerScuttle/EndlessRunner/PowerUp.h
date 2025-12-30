// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameplayEffect.h"
#include "PlayerClass.h"
#include "PowerUp.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ARabbitCharacter;

/**
 * Base powerup actor
 * Provides various temporary effects to the player via GAS
 */
UCLASS()
class SEWERSCUTTLE_API APowerUp : public AActor
{
	GENERATED_BODY()
	
public:
	APowerUp();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Get duration */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	float GetDuration() const { return Duration; }

	/** Collect this powerup */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void Collect(ARabbitCharacter* Player);

	/** Check if this power-up is valid for the given player class */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsValidForClass(EPlayerClass PlayerClass) const;

protected:
	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* CollisionSphere;

	/** Static mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Duration of powerup effect in seconds (0 = instant/one-time effect like extra life) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float Duration = 5.0f;

	/** Rotation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float RotationSpeed = 45.0f;

	/** Player classes that can use this power-up (empty = all classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Class Restrictions", meta = (ToolTip = "If empty, power-up is available for all classes. If populated, only listed classes can use this power-up."))
	TArray<EPlayerClass> AllowedClasses;

	/** Whether powerup has been collected */
	bool bCollected = false;

	/** Handle overlap */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Apply powerup effect to player */
	UFUNCTION(BlueprintImplementableEvent, Category = "!PowerUp")
	void OnPowerUpCollected(ARabbitCharacter* Player);

	/** Gameplay Effect class for permanent base stat modification (if set, applies permanent modification using ModificationValue) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!PowerUp|GAS")
	TSubclassOf<UGameplayEffect> PermanentStatModifierEffect;

	/** Gameplay Effect class for temporary multiplier (if set, applies temporary multiplier using ModificationValue for Duration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!PowerUp|GAS")
	TSubclassOf<UGameplayEffect> TemporaryMultiplierEffect;

	/** Stat type to modify (e.g., "BaseSpeed", "SpeedMultiplier", "CoinMultiplier") - must match SetByCaller tag in GE */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|GAS", meta = (ToolTip = "Stat name that matches the SetByCaller tag in your GameplayEffect (e.g., 'BaseSpeed', 'SpeedMultiplier')"))
	FName StatTypeToModify;

	/** Modification value - set this in the PowerUp, not in the GameplayEffect (additive for permanent, multiplier for temporary) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|GAS", meta = (ToolTip = "The value to apply. For permanent: adds to base stat. For temporary: multiplier amount (e.g., 0.5 = +50%)"))
	float ModificationValue = 0.0f;

protected:
	/** Apply powerup via GAS */
	void ApplyGASPowerUp(ARabbitCharacter* Player);
};

