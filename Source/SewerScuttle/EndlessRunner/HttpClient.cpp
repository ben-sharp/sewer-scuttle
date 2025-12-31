// Copyright Epic Games, Inc. All Rights Reserved.

#include "HttpClient.h"
#include "ConfigManager.h"
#include "AuthService.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformTime.h"

UHttpClient::UHttpClient()
{
	AuthService = nullptr;
	ConfigManager = nullptr;
}

void UHttpClient::Initialize(UAuthService* InAuthService)
{
	AuthService = InAuthService;
	ConfigManager = UConfigManager::Get();
	if (ConfigManager)
	{
		FString BaseUrl = ConfigManager->GetApiBaseUrl();
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: Initialize() - ConfigManager loaded, BaseUrl: '%s'"), *BaseUrl);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HttpClient: Initialize() - ConfigManager is NULL!"));
	}
}

void UHttpClient::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
}

void UHttpClient::ClearAuthToken()
{
	AuthToken.Empty();
}

FString UHttpClient::GetBaseUrl() const
{
	if (ConfigManager)
	{
		FString BaseUrl = ConfigManager->GetApiBaseUrl();
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: GetBaseUrl() - ConfigManager exists, returning: '%s'"), *BaseUrl);
		return BaseUrl;
	}
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: GetBaseUrl() - ConfigManager is NULL, using fallback: 'http://127.0.0.1:8000/api'"));
	return TEXT("http://127.0.0.1:8000/api");
}

void UHttpClient::Get(const FString& Endpoint, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError)
{
	MakeRequest(TEXT("GET"), Endpoint, TEXT(""), OnSuccess, OnError);
}

void UHttpClient::Post(const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError)
{
	MakeRequest(TEXT("POST"), Endpoint, RequestBody, OnSuccess, OnError);
}

void UHttpClient::Put(const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError)
{
	MakeRequest(TEXT("PUT"), Endpoint, RequestBody, OnSuccess, OnError);
}

void UHttpClient::Delete(const FString& Endpoint, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError)
{
	MakeRequest(TEXT("DELETE"), Endpoint, TEXT(""), OnSuccess, OnError);
}

void UHttpClient::MakeRequest(const FString& Verb, const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	// Build full URL
	FString BaseUrl = GetBaseUrl();
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: BaseUrl from ConfigManager: '%s'"), *BaseUrl);
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: Endpoint: '%s'"), *Endpoint);
	
	FString FullUrl = BaseUrl;
	if (!Endpoint.StartsWith(TEXT("/")))
	{
		FullUrl += TEXT("/");
	}
	FullUrl += Endpoint;
	
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: Full URL constructed: '%s'"), *FullUrl);

	Request->SetURL(FullUrl);
	Request->SetVerb(Verb);
	Request->SetTimeout(RequestTimeout);

	// Set headers
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	// Add authentication token if available
	bool bHasAuth = !AuthToken.IsEmpty();
	if (bHasAuth)
	{
		FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *AuthToken);
		Request->SetHeader(TEXT("Authorization"), AuthHeader);
	}

	// Set request body for POST/PUT
	if (!RequestBody.IsEmpty() && (Verb == TEXT("POST") || Verb == TEXT("PUT")))
	{
		Request->SetContentAsString(RequestBody);
	}

	// Log request details
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: [REQUEST] %s %s"), *Verb, *FullUrl);
	UE_LOG(LogTemp, Warning, TEXT("HttpClient: [REQUEST] BaseUrl: '%s', Endpoint: '%s', FullUrl: '%s'"), *BaseUrl, *Endpoint, *FullUrl);
	if (bHasAuth)
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [REQUEST] Authorization: Bearer %s..."), *AuthToken.Left(20));
	}
	if (!RequestBody.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [REQUEST] Body: %s"), *RequestBody);
	}

	// Bind response handler using lambda to capture callbacks
	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError, Verb, FullUrl](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
	{
		HandleRequestComplete(Req, Resp, bSuccess, OnSuccess, OnError, Verb, FullUrl);
	});

	// Process request
	Request->ProcessRequest();
}

void UHttpClient::HandleRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpResponse OnSuccess, FOnHttpError OnError, const FString& Verb, const FString& FullUrl)
{
	if (!Request.IsValid() || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [RESPONSE] Invalid request or response for %s %s"), *Verb, *FullUrl);
		if (OnError.IsBound())
		{
			OnError.Execute(0, TEXT("Invalid request or response"), TEXT(""));
		}
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	FString ResponseBody = Response->GetContentAsString();

	// Log response details
	UE_LOG(LogTemp, Log, TEXT("HttpClient: [RESPONSE] %s %s - Status: %d"), *Verb, *FullUrl, ResponseCode);
	
	// Log response body (truncate if too long to avoid log spam)
	if (!ResponseBody.IsEmpty())
	{
		FString LogBody = ResponseBody.Len() > 500 ? ResponseBody.Left(500) + TEXT("...") : ResponseBody;
		UE_LOG(LogTemp, Log, TEXT("HttpClient: [RESPONSE] Body: %s"), *LogBody);
	}

	// Handle 401 Unauthorized - try token refresh
	if (ResponseCode == 401 && AuthService)
	{
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [RESPONSE] Received 401 Unauthorized, attempting token refresh"));
		// AuthService will handle refresh and retry
		// For now, call error handler
		if (OnError.IsBound())
		{
			OnError.Execute(ResponseCode, TEXT("Unauthorized - token may be expired"), ResponseBody);
		}
		return;
	}

	// Handle 429 Too Many Requests - retry with backoff
	if (ResponseCode == 429)
	{
		FString RetryAfter = Response->GetHeader(TEXT("Retry-After"));
		int32 RetrySeconds = RetryAfter.IsEmpty() ? 5 : FCString::Atoi(*RetryAfter);
		
		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [RESPONSE] Rate limited (429), retry after %d seconds"), RetrySeconds);
		
		// Retry after delay (simplified - in production use a timer)
		// For now, call error handler
		if (OnError.IsBound())
		{
			OnError.Execute(ResponseCode, TEXT("Too many requests"), ResponseBody);
		}
		return;
	}

	// Handle success (2xx) or other errors
	if (bWasSuccessful && ResponseCode >= 200 && ResponseCode < 300)
	{
		UE_LOG(LogTemp, Log, TEXT("HttpClient: [RESPONSE] Success - %s %s completed successfully"), *Verb, *FullUrl);
		if (OnSuccess.IsBound())
		{
			OnSuccess.Execute(ResponseCode, ResponseBody);
		}
	}
	else
	{
		FString ErrorMessage = FString::Printf(TEXT("HTTP Error %d"), ResponseCode);
		if (ResponseCode == 403)
		{
			ErrorMessage = TEXT("Forbidden - account may be banned");
		}
		else if (ResponseCode >= 500)
		{
			ErrorMessage = TEXT("Server error");
		}

		UE_LOG(LogTemp, Warning, TEXT("HttpClient: [RESPONSE] Failed - %s %s - %s"), *Verb, *FullUrl, *ErrorMessage);
		if (OnError.IsBound())
		{
			OnError.Execute(ResponseCode, ErrorMessage, ResponseBody);
		}
	}
}

void UHttpClient::RetryRequest(const FString& Verb, const FString& Endpoint, const FString& RequestBody, const FOnHttpResponse& OnSuccess, const FOnHttpError& OnError, int32 RetryCount)
{
	// Exponential backoff: 1s, 2s, 4s
	float DelaySeconds = FMath::Pow(2.0f, RetryCount);
	
	UE_LOG(LogTemp, Log, TEXT("HttpClient: Retrying request (attempt %d) after %.1f seconds"), RetryCount + 1, DelaySeconds);
	
	// In a real implementation, use a timer here
	// For now, retry immediately (simplified)
	if (RetryCount < MaxRetries)
	{
		MakeRequest(Verb, Endpoint, RequestBody, OnSuccess, OnError);
	}
	else
	{
		if (OnError.IsBound())
		{
			OnError.Execute(0, TEXT("Max retries exceeded"), TEXT(""));
		}
	}
}

