// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpClient.generated.h"

DECLARE_DELEGATE_TwoParams(FOnHttpResponse, int32, const FString&);
DECLARE_DELEGATE_ThreeParams(FOnHttpError, int32, const FString&, const FString&);

/**
 * Lightweight wrapper for HTTP requests
 */
UCLASS()
class SEWERSCUTTLE_API UHttpClient : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	void Get(const FString& Endpoint, FOnHttpResponse OnSuccess, FOnHttpError OnError);
	void Post(const FString& Endpoint, const FString& Body, FOnHttpResponse OnSuccess, FOnHttpError OnError);

private:
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpResponse OnSuccess, FOnHttpError OnError);

	FString GetBaseUrl() const;
};

