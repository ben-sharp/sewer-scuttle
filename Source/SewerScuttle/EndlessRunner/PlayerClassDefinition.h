// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerClass.h"
#include "PlayerClassDefinition.generated.h"

/**
 * Data asset defining a player class configuration
 * Contains all class stats, perks, and display information
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UPlayerClassDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The class enum this definition represents (for dropdown selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EPlayerClass ClassType = EPlayerClass::Vanilla;

	/** Display order in class selection UI (lower = appears first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	int32 DisplayOrder = 0;

	/** Display name for the class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;

	/** Description text shown in class selection UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (MultiLine = true))
	FText Description;

	/** Whether this class is marked as "Coming Soon" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bComingSoon = false;

	/** Custom "Coming Soon" text (if empty, uses default "COMING SOON") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (EditCondition = "bComingSoon"))
	FText ComingSoonText;

	/** Class data (stats and perks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FPlayerClassData ClassData;
};

