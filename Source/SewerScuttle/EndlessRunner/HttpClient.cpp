// Copyright Epic Games, Inc. All Rights Reserved.

#include "HttpClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ConfigManager.h"
#include "SecureStorage.h"

void UHttpClient::Initialize()
{
}

void UHttpClient::Get(const FString& Endpoint, FOnHttpResponse OnSuccess, FOnHttpError OnError)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpClient::OnProcessRequestComplete, OnSuccess, OnError);
	Request->SetURL(GetBaseUrl() + Endpoint);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	
	FString TokenToSend = AuthToken;

	// 1. Try command line override (highest priority)
	FString CommandLineToken;
	if (FParse::Value(FCommandLine::Get(), TEXT("AuthToken="), CommandLineToken))
	{
		TokenToSend = CommandLineToken;
	}

	// 2. Try Editor dev override (only if not already set by command line)
	if (TokenToSend.IsEmpty() && GIsEditor)
	{
		if (UConfigManager* Config = UConfigManager::Get())
		{
			TokenToSend = Config->GetDevAuthToken();
		}
	}

	// 3. Try memory (set during session)
	// 4. Try persistent storage
	if (TokenToSend.IsEmpty())
	{
		TokenToSend = USecureStorage::Get()->Load(TEXT("auth_token"));
	}

	if (!TokenToSend.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *TokenToSend));
	}
	
	Request->ProcessRequest();
}

void UHttpClient::Post(const FString& Endpoint, const FString& Body, FOnHttpResponse OnSuccess, FOnHttpError OnError)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpClient::OnProcessRequestComplete, OnSuccess, OnError);
	Request->SetURL(GetBaseUrl() + Endpoint);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	
	FString TokenToSend = AuthToken;

	// 1. Try command line override (highest priority)
	FString CommandLineToken;
	if (FParse::Value(FCommandLine::Get(), TEXT("AuthToken="), CommandLineToken))
	{
		TokenToSend = CommandLineToken;
	}

	// 2. Try Editor dev override
	if (TokenToSend.IsEmpty() && GIsEditor)
	{
		if (UConfigManager* Config = UConfigManager::Get())
		{
			TokenToSend = Config->GetDevAuthToken();
		}
	}

	// 3. Try memory / storage
	if (TokenToSend.IsEmpty())
	{
		TokenToSend = USecureStorage::Get()->Load(TEXT("auth_token"));
	}

	if (!TokenToSend.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *TokenToSend));
	}

	Request->SetContentAsString(Body);
	Request->ProcessRequest();
}

void UHttpClient::OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnHttpResponse OnSuccess, FOnHttpError OnError)
{
	if (bWasSuccessful && Response.IsValid())
	{
		OnSuccess.ExecuteIfBound(Response->GetResponseCode(), Response->GetContentAsString());
	}
	else
	{
		int32 Code = Response.IsValid() ? Response->GetResponseCode() : 0;
		FString Msg = bWasSuccessful ? TEXT("Request failed") : TEXT("Connection failed");
		FString Body = Response.IsValid() ? Response->GetContentAsString() : TEXT("");
		OnError.ExecuteIfBound(Code, Msg, Body);
	}
}

FString UHttpClient::GetBaseUrl() const
{
	UConfigManager* Config = UConfigManager::Get();
	return Config ? Config->GetApiBaseUrl() : TEXT("http://backend.test/api");
}

