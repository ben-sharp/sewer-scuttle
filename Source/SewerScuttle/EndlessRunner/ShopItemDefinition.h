// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PlayerClass.h"
#include "BaseContentDefinition.h"
#include "TrackPieceDefinition.h"
#include "ShopItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EShopItemType : uint8
{
	PowerUp,
	StatModifier,
	Collectible,
	Custom
};

/**
 * Data asset defining an item available in the shop or as a boss reward
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UShopItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString ItemName;

	/** Cost in run currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 BaseCost = 100;

	/** Item type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	EShopItemType ItemType = EShopItemType::PowerUp;

	/** Gameplay Tag associated with the effect (e.g., SpeedMultiplier, ExtraLife) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag EffectTag;

	/** Value to apply for the effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float EffectValue = 1.0f;

	/** Description shown in the shop UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = true))
	FText Description;

	/** Icon for the shop UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Classes allowed to see/purchase this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
	TArray<EPlayerClass> AllowedClasses;

	/** Tiers where this item is available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
	TArray<ETrackTier> DifficultyAvailability;

	/** Whether this item can be a boss reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filtering")
	bool bCanBeBossReward = true;
};

