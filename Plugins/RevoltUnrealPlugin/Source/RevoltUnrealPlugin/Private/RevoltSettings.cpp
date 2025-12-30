// Copyright Epic Games, Inc. All Rights Reserved.

#include "RevoltSettings.h"

URevoltSettings::URevoltSettings()
{
	CategoryName = TEXT("Plugins");
}

FName URevoltSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

