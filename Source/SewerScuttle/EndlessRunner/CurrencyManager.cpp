// Copyright Epic Games, Inc. All Rights Reserved.

#include "CurrencyManager.h"
#include "WebServerInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

UCurrencyManager::UCurrencyManager()
{
	Currency = 0;
	WebServerInterface = nullptr;
}

void UCurrencyManager::Initialize()
{
	// Create web server interface
	WebServerInterface = NewObject<UWebServerInterface>(this);

	// Load currency from local storage
	LoadCurrencyLocal();
}

void UCurrencyManager::AddCurrency(int32 Amount)
{
	Currency += Amount;
	SaveCurrencyLocal();
}

bool UCurrencyManager::SpendCurrency(int32 Amount)
{
	if (Currency >= Amount)
	{
		Currency -= Amount;
		SaveCurrencyLocal();
		return true;
	}
	return false;
}

void UCurrencyManager::SaveCoins()
{
	// Save locally first
	SaveCurrencyLocal();

	// Save to web server (async)
	if (WebServerInterface)
	{
		WebServerInterface->SaveCurrency(Currency);
	}
}

void UCurrencyManager::LoadCurrency()
{
	// Load from local storage first
	LoadCurrencyLocal();

	// Load from web server (async)
	if (WebServerInterface)
	{
		WebServerInterface->LoadCurrency();
	}
}

bool UCurrencyManager::PurchaseItem(const FString& ItemId, int32 Price)
{
	if (SpendCurrency(Price))
	{
		// Notify web server of purchase
		if (WebServerInterface)
		{
			WebServerInterface->PurchaseItem(ItemId, Price);
		}
		return true;
	}
	return false;
}

void UCurrencyManager::SaveCurrencyLocal()
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Currency.dat");
	FString CurrencyString = FString::Printf(TEXT("%d"), Currency);
	FFileHelper::SaveStringToFile(CurrencyString, *SavePath);
}

void UCurrencyManager::LoadCurrencyLocal()
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Currency.dat");
	FString CurrencyString;
	
	if (FFileHelper::LoadFileToString(CurrencyString, *SavePath))
	{
		Currency = FCString::Atoi(*CurrencyString);
	}
	else
	{
		Currency = 0;
	}
}

