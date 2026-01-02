// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CurrencyManager.generated.h"

class UWebServerInterface;

/**
 * Manages local currency and interfaces with web server
 */
UCLASS()
class SEWERSCUTTLE_API UCurrencyManager : public UObject
{
	GENERATED_BODY()

public:
	UCurrencyManager();

	/** Initialize currency manager */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void Initialize();

	/** Get current currency */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrency() const { return Currency; }

	/** Add currency */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCurrency(int32 Amount);

	/** Spend currency */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCurrency(int32 Amount);

	/** Save currency to web server */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SaveCoins();

	/** Load currency from web server */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void LoadCurrency();

	/** Purchase an item */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool PurchaseItem(const FString& ItemId, int32 Price);

protected:
	/** Web server interface */
	UPROPERTY()
	UWebServerInterface* WebServerInterface;

	/** Current currency amount */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Currency")
	int32 Currency = 0;

	/** Save currency locally */
	void SaveCurrencyLocal();

	/** Load currency locally */
	void LoadCurrencyLocal();
};

