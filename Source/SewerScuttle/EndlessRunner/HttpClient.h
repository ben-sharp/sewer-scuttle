// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpClient.generated.h"

class UConfigManager;
class UAuthService;
class IHttpRequest;
class IHttpResponse;
typedef TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestPtr;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

// Non-dynamic delegates for HTTP callbacks (support CreateUObject/CreateLambda)
DECLARE_DELEGATE_TwoParams(FOnHttpResponse, int32 /*ResponseCode*/, const FString& /*ResponseBody*/);
DECLARE_DELEGATE_ThreeParams(FOnHttpError, int32 /*ResponseCode*/, const FString& /*ErrorMessage*/, const FString& /*ResponseBody*/);

/**
 * HTTP client wrapper with automatic token injection
 * Handles all API communication with the Laravel backend
 */
UCLASS()
class SEWERSCUTTLE_API UHttpClient : public UObject
{
	GENERATED_BODY()

public:
	UHttpClient();

	/** Initialize HTTP client */
	UFUNCTION(BlueprintCallable, Category = "HTTP")
	void Initialize(UAuthService* InAuthService);

	/** Set authentication token (called by AuthService) */
	void SetAuthToken(const FString& Token);

	/** Clear authentication token */
	void ClearAuthToken();

	/** GET request */
	void Get(const FString& Endpoint, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError);

	/** POST request */
	void Post(const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError);

	/** PUT request */
	void Put(const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError);

	/** DELETE request */
	void Delete(const FString& Endpoint, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError);

	/** Get base URL */
	UFUNCTION(BlueprintPure, Category = "HTTP")
	FString GetBaseUrl() const;

private:
	/** Make HTTP request (internal) */
	void MakeRequest(const FString& Verb, const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError);

	/** Handle HTTP response completion */
	void HandleRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpResponse OnSuccess, FOnHttpError OnError, const FString& Verb, const FString& FullUrl);

	/** Retry request with exponential backoff */
	void RetryRequest(const FString& Verb, const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError, int32 RetryCount);

	/** Authentication token */
	FString AuthToken;

	/** Auth service reference (for token refresh) */
	UPROPERTY()
	UAuthService* AuthService;

	/** Config manager reference */
	UPROPERTY()
	UConfigManager* ConfigManager;

	/** Request timeout in seconds */
	UPROPERTY()
	float RequestTimeout = 30.0f;

	/** Maximum retry attempts */
	UPROPERTY()
	int32 MaxRetries = 3;
};

