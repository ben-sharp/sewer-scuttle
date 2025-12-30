// Copyright Epic Games, Inc. All Rights Reserved.

#include "SClassSelectionWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "../EndlessRunner/EndlessRunnerGameMode.h"
#include "../EndlessRunner/PlayerClassDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

void SClassSelectionWidget::Construct(const FArguments& InArgs)
{
	OnClassSelected = InArgs._OnClassSelected;

	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Select Your Class")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]

		// Class selection buttons (dynamically generated from data assets)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(20.0f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ClassListContainer, SVerticalBox)
				]
			]
		]
	];

	// Populate class buttons from data assets
	// Will be refreshed after widget is shown to ensure GameMode is available
}

void SClassSelectionWidget::PopulateClassButtons()
{
	if (!ClassListContainer.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: ClassListContainer is not valid"));
		return;
	}

	ClassListContainer->ClearChildren();

	// Use AssetRegistry to find all PlayerClassDefinition data assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all PlayerClassDefinition assets in /Game/EndlessRunner/Classes
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UPlayerClassDefinition::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(TEXT("/Game/EndlessRunner/Classes"));
	Filter.bRecursivePaths = true;
	
	AssetRegistry.GetAssets(Filter, AssetDataList);
	
	UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: Found %d PlayerClassDefinition assets in AssetRegistry"), AssetDataList.Num());

	if (AssetDataList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: No PlayerClassDefinition assets found in /Game/EndlessRunner/Classes"));
		return;
	}

	// Load all assets and sort by DisplayOrder
	TArray<UPlayerClassDefinition*> ClassDefinitions;
	for (const FAssetData& AssetData : AssetDataList)
	{
		if (UPlayerClassDefinition* ClassDef = Cast<UPlayerClassDefinition>(AssetData.GetAsset()))
		{
			if (ClassDef)
			{
				ClassDefinitions.Add(ClassDef);
			}
		}
	}

	// Sort by DisplayOrder (lower = first)
	ClassDefinitions.Sort([](const UPlayerClassDefinition& A, const UPlayerClassDefinition& B)
	{
		return A.DisplayOrder < B.DisplayOrder;
	});

	UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: Loaded %d class definitions, sorted by DisplayOrder"), ClassDefinitions.Num());

	for (UPlayerClassDefinition* ClassDef : ClassDefinitions)
	{
		if (!ClassDef)
		{
			UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: Found null class definition, skipping"));
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("SClassSelectionWidget: Adding class button for %s (Type: %d, ComingSoon: %d)"), 
			*ClassDef->DisplayName.ToString(), (int32)ClassDef->ClassType, ClassDef->bComingSoon);

		// Create button for this class
		ClassListContainer->AddSlot()
		.AutoHeight()
		.Padding(10.0f)
		[
			SNew(SButton)
			.ContentPadding(FMargin(20.0f, 15.0f))
			.IsEnabled(ClassDef && !ClassDef->bComingSoon)
			.OnClicked(this, &SClassSelectionWidget::OnClassButtonClicked, ClassDef)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 10)
				[
					SNew(STextBlock)
					.Text(ClassDef->DisplayName)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(ClassDef->bComingSoon ? FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)) : FSlateColor::UseForeground())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(ClassDef->bComingSoon ? 0 : 1)
					+ SWidgetSwitcher::Slot()
					[
						SNew(STextBlock)
						.Text(ClassDef->ComingSoonText.IsEmpty() ? FText::FromString(TEXT("COMING SOON")) : ClassDef->ComingSoonText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f))) // Gold
					]
					+ SWidgetSwitcher::Slot()
					[
						SNew(STextBlock)
						.Text(ClassDef->Description)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
				]
			]
		];
	}
	
	// Invalidate the widget to force a redraw
	if (ClassListContainer.IsValid())
	{
		ClassListContainer->Invalidate(EInvalidateWidget::LayoutAndVolatility);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SClassSelectionWidget: Finished populating %d class buttons"), ClassDefinitions.Num());
}

FReply SClassSelectionWidget::OnClassButtonClicked(UPlayerClassDefinition* SelectedClassDef)
{
	if (OnClassSelected.IsBound() && SelectedClassDef)
	{
		OnClassSelected.Execute(SelectedClassDef->ClassType);
	}
	return FReply::Handled();
}


void SClassSelectionWidget::RefreshClassButtons()
{
	PopulateClassButtons();
}

