// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "../EndlessRunner/WebServerInterface.h"

DECLARE_DELEGATE_OneParam(FOnTrackSelected, int32);

/**
 * Track selection widget for choosing a track at the start of each tier
 */
class SEWERSCUTTLE_API STrackSelectionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STrackSelectionWidget)
	{}
		SLATE_EVENT(FOnTrackSelected, OnTrackSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update track selection data */
	void UpdateTracks(const FTrackSelectionData& SelectionData);

	/** Delegate for track selection */
	FOnTrackSelected OnTrackSelected;

private:
	/** Handle track button click */
	FReply OnTrackButtonClicked(int32 TrackIndex);

	/** Container for track cards */
	TSharedPtr<class SHorizontalBox> TrackCardsContainer;

	/** Track selection data */
	FTrackSelectionData CurrentSelectionData;

	/** Tier text block */
	TSharedPtr<class STextBlock> TierTextBlock;
};

