// Copyright Epic Games, Inc. All Rights Reserved.

#include "QueryOptions.h"

FQueryOptions FQueryOptions::ParseFromQueryString(const FString& QueryString)
{
	FQueryOptions Options;

	// Parse query string parameters
	TArray<FString> Params;
	QueryString.ParseIntoArray(Params, TEXT("&"));

	for (const FString& Param : Params)
	{
		FString Key, Value;
		if (Param.Split(TEXT("="), &Key, &Value))
		{
			Key = Key.ToLower();
			Value = Value.ToLower();

			// Parse depth level
			if (Key == TEXT("depth"))
			{
				if (Value == TEXT("basic"))
				{
					Options.ApplyDepthLevel(EQueryDepth::Basic);
				}
				else if (Value == TEXT("standard"))
				{
					Options.ApplyDepthLevel(EQueryDepth::Standard);
				}
				else if (Value == TEXT("deep"))
				{
					Options.ApplyDepthLevel(EQueryDepth::Deep);
				}
				else if (Value == TEXT("full"))
				{
					Options.ApplyDepthLevel(EQueryDepth::Full);
				}
			}
			// Parse boolean flags
			else if (Key == TEXT("properties"))
			{
				Options.bIncludeProperties = (Value == TEXT("true") || Value == TEXT("1"));
			}
			else if (Key == TEXT("functions"))
			{
				Options.bIncludeFunctions = (Value == TEXT("true") || Value == TEXT("1"));
			}
			else if (Key == TEXT("graph"))
			{
				Options.bIncludeGraphNodes = (Value == TEXT("true") || Value == TEXT("1"));
			}
			else if (Key == TEXT("defaults"))
			{
				Options.bIncludeDefaultValues = (Value == TEXT("true") || Value == TEXT("1"));
			}
			else if (Key == TEXT("connections"))
			{
				Options.bIncludeConnections = (Value == TEXT("true") || Value == TEXT("1"));
			}
			else if (Key == TEXT("components"))
			{
				Options.bIncludeComponents = (Value == TEXT("true") || Value == TEXT("1"));
			}
		}
	}

	return Options;
}

void FQueryOptions::ApplyDepthLevel(EQueryDepth Depth)
{
	DepthLevel = Depth;

	switch (Depth)
	{
	case EQueryDepth::Basic:
		bIncludeProperties = false;
		bIncludeFunctions = false;
		bIncludeGraphNodes = false;
		bIncludeDefaultValues = false;
		bIncludeConnections = false;
		bIncludeComponents = false;
		break;

	case EQueryDepth::Standard:
		bIncludeProperties = true;
		bIncludeFunctions = true;
		bIncludeGraphNodes = false;
		bIncludeDefaultValues = false;
		bIncludeConnections = false;
		bIncludeComponents = true;
		break;

	case EQueryDepth::Deep:
		bIncludeProperties = true;
		bIncludeFunctions = true;
		bIncludeGraphNodes = false;
		bIncludeDefaultValues = true;
		bIncludeConnections = false;
		bIncludeComponents = true;
		break;

	case EQueryDepth::Full:
		bIncludeProperties = true;
		bIncludeFunctions = true;
		bIncludeGraphNodes = true;
		bIncludeDefaultValues = true;
		bIncludeConnections = true;
		bIncludeComponents = true;
		break;
	}
}

