#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

class URevoltMenuConfig;
class STextBlock;
class SBorder;

/**
 * Animated button for the menu.
 */
class REVOLTMENUFRAMEWORK_API SRevoltMenuButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRevoltMenuButton)
		: _Text()
		, _BaseColor(FLinearColor::White)
		, _OnClicked()
	{}
	SLATE_ARGUMENT(FText, Text)
	SLATE_ARGUMENT(FLinearColor, BaseColor)
	SLATE_EVENT(FOnClicked, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	FReply OnButtonClicked();
	void OnButtonHovered();
	void OnButtonUnhovered();

	FOnClicked OnClickedDelegate;
	FLinearColor BaseColor;
	
	// Animation
	float CurrentScale;
	float TargetScale;
	float CurrentBrightness;
	float TargetBrightness;

	TSharedPtr<SBorder> BackgroundBorder;
	TSharedPtr<STextBlock> TextBlock;
};

/**
 * Slate implementation of the Main Menu.
 */
class REVOLTMENUFRAMEWORK_API SRevoltMainMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRevoltMainMenu)
	{}
	/** The config asset defining logo and levels */
	SLATE_ARGUMENT(URevoltMenuConfig*, MenuConfig)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	/** Rebuilds the UI for the Main Menu state */
	void ConstructMainMenu();

	/** Rebuilds the UI for the Settings state */
	void ConstructSettings();

	/** Rebuilds the UI for the Level Select state */
	void ConstructLevelSelect();

	/** Callback for Start Game button */
	FReply OnStartGameClicked();

	/** Callback for Load Level button */
	FReply OnLoadLevelClicked();

	/** Callback for Settings button */
	FReply OnSettingsClicked();

	/** Callback for Back button in Settings */
	FReply OnBackClicked();

	/** Callback for Exit Game button */
	FReply OnExitGameClicked();

	/** Callback for Website button */
	FReply OnWebsiteButtonClicked();

	/** Callback for Resolution Change */
	void OnResolutionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo);

	/** Callback for Fullscreen Change */
	void OnFullscreenChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo);

	/** Applies current settings */
	FReply OnApplySettingsClicked();

	/** Callback when a specific level is clicked in the list */
	FReply OnSelectLevelClicked(TSoftObjectPtr<UWorld> LevelToLoad);

	/** Pointer to the config */
	URevoltMenuConfig* MenuConfig;

	/** Container for swapping menu content */
	TSharedPtr<SVerticalBox> MenuContainer;

	/** Resolution Options */
	TArray<TSharedPtr<FString>> ResolutionOptions;

	/** Fullscreen Options */
	TArray<TSharedPtr<FString>> FullscreenOptions;

	/** Current Selection State */
	TSharedPtr<FString> CurrentResolution;
	TSharedPtr<FString> CurrentFullscreenMode;

	/** Animation state */
	float MenuOpacity = 0.0f;
	
	/** Animation tick */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** Opacity callback for SBorder (FLinearColor) */
	FLinearColor GetMenuOpacity() const;

	/** Opacity callback for SImage (FSlateColor) */
	FSlateColor GetMenuOpacitySlate() const;
};

