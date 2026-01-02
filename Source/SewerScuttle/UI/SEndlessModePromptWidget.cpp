// Copyright Epic Games, Inc. All Rights Reserved.

#include "SEndlessModePromptWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Styling/SlateColor.h"
#include "Styling/CoreStyle.h"

void SEndlessModePromptWidget::Construct(const FArguments& InArgs)
{
	OnEndlessModeSelected = InArgs._OnEndlessModeSelected;
	OnEndlessModeDeclined = InArgs._OnEndlessModeDeclined;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 20)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("CONGRATULATIONS!")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FSlateColor(FLinearColor::Green))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 40)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("You have defeated the final boss. Continue into the endless sewers?")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(10)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Go Endless")))
					.OnClicked(this, &SEndlessModePromptWidget::OnEndlessClicked)
					.ContentPadding(FMargin(20, 10))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(10)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Finish Run")))
					.OnClicked(this, &SEndlessModePromptWidget::OnQuitClicked)
					.ContentPadding(FMargin(20, 10))
				]
			]
		]
	];
}

FReply SEndlessModePromptWidget::OnEndlessClicked()
{
	if (OnEndlessModeSelected.IsBound())
	{
		OnEndlessModeSelected.Execute();
	}
	return FReply::Handled();
}

FReply SEndlessModePromptWidget::OnQuitClicked()
{
	if (OnEndlessModeDeclined.IsBound())
	{
		OnEndlessModeDeclined.Execute();
	}
	return FReply::Handled();
}

