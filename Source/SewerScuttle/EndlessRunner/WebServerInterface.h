// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebServerInterface.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCurrencyLoaded, int32, Currency);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLeaderboardLoaded, TArray<FString>, Scores);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnScorePosted, bool, Success);

/**
 * Interface for web server communication
 * Stubbed implementation ready for HTTP client integration
 */
UCLASS()
class SEWERSCUTTLE_API UWebServerInterface : public UObject
{
	GENERATED_BODY()

public:
	UWebServerInterface();

	/** Web server base URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Web Server")
	FString BaseURL = TEXT("http://localhost:3000/api");

	/** Save currency to web server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SaveCurrency(int32 Currency);

	/** Load currency from web server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void LoadCurrency();

	/** Post score to leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void PostScore(int32 Score, float Distance);

	/** Get leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void GetLeaderboard(int32 TopCount = 10);

	/** Purchase item */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void PurchaseItem(const FString& ItemId, int32 Price);

	/** Set callback for currency loaded */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnCurrencyLoaded(FOnCurrencyLoaded Callback) { OnCurrencyLoaded = Callback; }

	/** Set callback for leaderboard loaded */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnLeaderboardLoaded(FOnLeaderboardLoaded Callback) { OnLeaderboardLoaded = Callback; }

	/** Set callback for score posted */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnScorePosted(FOnScorePosted Callback) { OnScorePosted = Callback; }

protected:
	/** Callback for currency loaded */
	UPROPERTY()
	FOnCurrencyLoaded OnCurrencyLoaded;

	/** Callback for leaderboard loaded */
	UPROPERTY()
	FOnLeaderboardLoaded OnLeaderboardLoaded;

	/** Callback for score posted */
	UPROPERTY()
	FOnScorePosted OnScorePosted;

	/** Make HTTP request (stubbed - ready for HTTP module integration) */
	void MakeHTTPRequest(const FString& Endpoint, const FString& Method, const FString& Body = TEXT(""));

	/** Handle HTTP response (stubbed) */
	void HandleHTTPResponse(const FString& Response, const FString& Endpoint);
};

