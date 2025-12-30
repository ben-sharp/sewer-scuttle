// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Depth level for query extraction
 */
UENUM()
enum class EQueryDepth : uint8
{
	Basic,      // Name, path, class only
	Standard,   // + properties and functions
	Deep,       // + default values and components
	Full        // + graph nodes and connections
};

/**
 * Query options for controlling data extraction depth
 * Parsed from URL query parameters
 */
struct FQueryOptions
{
	/** Depth level for extraction */
	EQueryDepth DepthLevel = EQueryDepth::Standard;
	
	/** Include UPROPERTY data */
	bool bIncludeProperties = true;
	
	/** Include UFUNCTION signatures */
	bool bIncludeFunctions = true;
	
	/** Include Blueprint graph nodes */
	bool bIncludeGraphNodes = false;
	
	/** Include property default values */
	bool bIncludeDefaultValues = false;
	
	/** Include node pin connections */
	bool bIncludeConnections = false;

	/** Include component data for actors */
	bool bIncludeComponents = true;

	/** Default constructor */
	FQueryOptions() = default;

	/**
	 * Parse query options from URL query string
	 * @param QueryString URL query parameters (e.g., "depth=full&properties=true")
	 * @return Parsed query options
	 */
	static FQueryOptions ParseFromQueryString(const FString& QueryString);

	/**
	 * Apply depth level preset
	 * @param Depth The depth level to apply
	 */
	void ApplyDepthLevel(EQueryDepth Depth);
};

