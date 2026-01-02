// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthService.h"
#include "HttpClient.h"
#include "SecureStorage.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UAuthService::Login(const FUserCredentials& Credentials, FOnAuthSuccess OnSuccess, FOnAuthError OnError)
{
    // Implementation
}

void UAuthService::Register(const FString& Name, const FString& Email, const FString& Password, FOnAuthSuccess OnSuccess, FOnAuthError OnError)
{
    // Implementation
}

void UAuthService::Logout()
{
    AuthToken = TEXT("");
    USecureStorage::Get()->Remove(TEXT("auth_token"));
}

bool UAuthService::IsAuthenticated() const
{
    return !AuthToken.IsEmpty();
}

FString UAuthService::GetToken() const
{
    return AuthToken;
}

