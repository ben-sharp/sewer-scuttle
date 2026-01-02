// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthModels.h"
#include "AuthService.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAuthSuccess, const FAuthResponse&, Response);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAuthError, const FString&, ErrorMessage);

UCLASS(BlueprintType)
class SEWERSCUTTLE_API UAuthService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Auth")
	void Login(const FUserCredentials& Credentials, FOnAuthSuccess OnSuccess, FOnAuthError OnError);

	UFUNCTION(BlueprintCallable, Category = "Auth")
	void Register(const FString& Name, const FString& Email, const FString& Password, FOnAuthSuccess OnSuccess, FOnAuthError OnError);

	UFUNCTION(BlueprintCallable, Category = "Auth")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "Auth")
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintPure, Category = "Auth")
	FString GetToken() const;

private:
	FString AuthToken;
	FAuthUserData CurrentUser;
};

