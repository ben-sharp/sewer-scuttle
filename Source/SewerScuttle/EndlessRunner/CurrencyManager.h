// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CurrencyManager.generated.h"

class UWebServerInterface;

/**
 * Manages persistent coins (wallet) - server-backed currency that persists across sessions
 */
UCLASS()
class SEWERSCUTTLE_API UCurrencyManager : public UObject
{
	GENERATED_BODY()

public:
	UCurrencyManager();

	/** Initialize currency manager (loads coins from server) */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	void Initialize();

	/** Get current coins (persistent wallet) */
	UFUNCTION(BlueprintPure, Category = "Coins")
	int32 GetCoins() const { return Coins; }

	/** Add coins to persistent wallet (server-backed) */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	void AddCoins(int32 Amount);

	/** Spend coins from persistent wallet */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	bool SpendCoins(int32 Amount);

	/** Save coins to web server */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	void SaveCoins();

	/** Load coins from web server */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	void LoadCoins();

	/** Purchase an item with coins */
	UFUNCTION(BlueprintCallable, Category = "Coins")
	bool PurchaseItem(const FString& ItemId, int32 Price);

	// Legacy functions for backwards compatibility (deprecated - use Coins versions)
	UFUNCTION(BlueprintPure, Category = "Coins", meta = (DeprecatedFunction, DeprecationMessage = "Use GetCoins() instead"))
	int32 GetCurrency() const { return Coins; }

	UFUNCTION(BlueprintCallable, Category = "Coins", meta = (DeprecatedFunction, DeprecationMessage = "Use AddCoins() instead"))
	void AddCurrency(int32 Amount) { AddCoins(Amount); }

	UFUNCTION(BlueprintCallable, Category = "Coins", meta = (DeprecatedFunction, DeprecationMessage = "Use SpendCoins() instead"))
	bool SpendCurrency(int32 Amount) { return SpendCoins(Amount); }

protected:
	/** Web server interface */
	UPROPERTY()
	UWebServerInterface* WebServerInterface;

	/** Current coins amount (persistent wallet, server-backed) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins")
	int32 Coins = 0;

	/** Save coins locally (temporary cache until server sync) */
	void SaveCoinsLocal();

	/** Load coins locally (temporary cache, server is source of truth) */
	void LoadCoinsLocal();
};

