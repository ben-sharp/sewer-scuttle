#include "SRevoltMainMenu.h"
#include "RevoltMenuConfig.h"
#include "RevoltMenuFramework.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h" // Added required include for FSlateImageBrush
#include "Misc/PackageName.h" // Added required include for FPackageName
#include "GameFramework/PlayerController.h"
#include "SlateOptMacros.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "RevoltMenu"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SRevoltMainMenu::Construct(const FArguments& InArgs)
{
	MenuConfig = InArgs._MenuConfig;
	MenuOpacity = 0.0f; // Start transparent

	if (!MenuConfig)
	{
		return;
	}

	// Ensure soft object pointers are loaded for cooked builds
	if (!MenuConfig->StartGameLevel.IsNull())
	{
		MenuConfig->StartGameLevel.LoadSynchronous();
	}

	for (auto& LevelPtr : MenuConfig->SelectableLevels)
	{
		if (!LevelPtr.IsNull())
		{
			LevelPtr.LoadSynchronous();
		}
	}

	// Music tracks are already USoundBase pointers, no loading needed
	if (MenuConfig->MusicPlaylist.Num() > 0)
	{
		FRevoltMenuFrameworkModule::LogToFileAndScreen(FString::Printf(TEXT("Found %d music tracks"), MenuConfig->MusicPlaylist.Num()));
	}

	// Initialize Settings Options
	ResolutionOptions.Add(MakeShared<FString>(TEXT("1920x1080")));
	ResolutionOptions.Add(MakeShared<FString>(TEXT("1280x720")));
	ResolutionOptions.Add(MakeShared<FString>(TEXT("2560x1440")));
	ResolutionOptions.Add(MakeShared<FString>(TEXT("3840x2160")));
	
	FullscreenOptions.Add(MakeShared<FString>(TEXT("Fullscreen")));
	FullscreenOptions.Add(MakeShared<FString>(TEXT("Windowed Fullscreen")));
	FullscreenOptions.Add(MakeShared<FString>(TEXT("Windowed")));

	// Set initial selection based on current settings
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		FIntPoint Res = Settings->GetScreenResolution();
		FString CurrentResStr = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
		
		// Find matching option or default to first
		CurrentResolution = ResolutionOptions[0];
		for (auto& Option : ResolutionOptions)
		{
			if (*Option == CurrentResStr)
			{
				CurrentResolution = Option;
				break;
			}
		}

		EWindowMode::Type Mode = Settings->GetFullscreenMode();
		if (Mode == EWindowMode::Fullscreen) CurrentFullscreenMode = FullscreenOptions[0];
		else if (Mode == EWindowMode::WindowedFullscreen) CurrentFullscreenMode = FullscreenOptions[1];
		else CurrentFullscreenMode = FullscreenOptions[2];
	}

	ChildSlot
	[
		SNew(SOverlay)
		
		// Full screen background gradient (subtle overlay on top of 3D world)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.ColorAndOpacity(this, &SRevoltMainMenu::GetMenuOpacitySlate) // Fade in (FSlateColor)
			.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.3f))
		]

		// Menu Container (Left Aligned)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		.Padding(100, 0, 0, 0)
		[
			SNew(SBackgroundBlur)
			.BlurStrength(15.0f)
			.Padding(0)
			[
				SNew(SBorder)
				.BorderImage(new FSlateColorBrush(FLinearColor(0.02f, 0.02f, 0.02f, 0.8f)))
				.Padding(FMargin(50, 100))
				.ColorAndOpacity(this, &SRevoltMainMenu::GetMenuOpacity) // Fade in content
				[
					SAssignNew(MenuContainer, SVerticalBox)
				]
			]
		]
	];

	// Build the initial menu
	ConstructMainMenu();
}

void SRevoltMainMenu::ConstructMainMenu()
{
	MenuContainer->ClearChildren();

	// Default Style Colors
	FLinearColor PrimaryColor = FLinearColor(1.0f, 0.2f, 0.4f, 1.0f);
	if (MenuConfig) PrimaryColor = MenuConfig->PrimaryColor;

	// Prepare Logo
	TSharedPtr<SWidget> LogoWidget;
	if (MenuConfig && MenuConfig->MenuLogo)
	{
		// Create a proper brush resource that will work in cooked builds
		FSlateBrush LogoBrush;
		LogoBrush.SetResourceObject(MenuConfig->MenuLogo);
		LogoBrush.ImageSize = FVector2D(500, 200);

		LogoWidget = SNew(SImage)
			.Image(&LogoBrush);
	}
	else
	{
		FText Title = (MenuConfig) ? MenuConfig->GameTitle : LOCTEXT("DefaultTitle", "Revolt Game");
		LogoWidget = SNew(STextBlock)
			.Text(Title)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 64))
			.ColorAndOpacity(FLinearColor::White)
			.ShadowOffset(FVector2D(2, 2))
			.ShadowColorAndOpacity(FLinearColor::Black)
			.Justification(ETextJustify::Center);
	}

	FText FooterText = (MenuConfig) ? MenuConfig->FooterText : LOCTEXT("Footer", "Powered by Revolt Framework");

	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 80)
		.HAlign(HAlign_Left)
		[
			LogoWidget.ToSharedRef()
		];

	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 15)
		[
			SNew(SRevoltMenuButton)
			.Text(LOCTEXT("StartGame", "START GAME"))
			.BaseColor(PrimaryColor)
			.OnClicked(this, &SRevoltMainMenu::OnStartGameClicked)
		];

	// Only show Load Level button if there are levels available
	if (MenuConfig && MenuConfig->SelectableLevels.Num() > 0)
	{
		MenuContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 15)
			[
				SNew(SRevoltMenuButton)
				.Text(LOCTEXT("LoadLevel", "LOAD LEVEL"))
				.BaseColor(FLinearColor::White)
				.OnClicked(this, &SRevoltMainMenu::OnLoadLevelClicked)
			];
	}

	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 15)
		[
			SNew(SRevoltMenuButton)
			.Text(LOCTEXT("Settings", "SETTINGS"))
			.BaseColor(FLinearColor::White)
			.OnClicked(this, &SRevoltMainMenu::OnSettingsClicked)
		];

	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 15)
		[
			SNew(SRevoltMenuButton)
			.Text(LOCTEXT("ExitGame", "EXIT GAME"))
			.BaseColor(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
			.OnClicked(this, &SRevoltMainMenu::OnExitGameClicked)
		];

	MenuContainer->AddSlot()
		.FillHeight(1.0f);

	// Website button (shown only if enabled in config)
	if (MenuConfig && MenuConfig->bShowWebsiteButton)
	{
		MenuContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 15)
			[
				SNew(SRevoltMenuButton)
				.Text(MenuConfig->WebsiteButtonText)
				.BaseColor(FLinearColor(0.2f, 0.6f, 1.0f, 1.0f)) // Blue color for website links
				.OnClicked(this, &SRevoltMainMenu::OnWebsiteButtonClicked)
			];
	}

	MenuContainer->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FooterText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity(FLinearColor(1,1,1,0.3f))
		];
}

void SRevoltMainMenu::ConstructSettings()
{
	MenuContainer->ClearChildren();
	
	FLinearColor PrimaryColor = FLinearColor(1.0f, 0.2f, 0.4f, 1.0f);
	if (MenuConfig) PrimaryColor = MenuConfig->PrimaryColor;

	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 50)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsHeader", "SETTINGS"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			.ColorAndOpacity(FLinearColor::White)
		];

	// Resolution Row
	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Resolution", "Resolution"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ResolutionOptions)
				.InitiallySelectedItem(CurrentResolution)
				.OnSelectionChanged(this, &SRevoltMainMenu::OnResolutionChanged)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return CurrentResolution.IsValid() ? FText::FromString(*CurrentResolution) : FText::FromString("Select..."); })
				]
			]
		];

	// Fullscreen Row
	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 10)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Fullscreen", "Window Mode"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&FullscreenOptions)
				.InitiallySelectedItem(CurrentFullscreenMode)
				.OnSelectionChanged(this, &SRevoltMainMenu::OnFullscreenChanged)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return CurrentFullscreenMode.IsValid() ? FText::FromString(*CurrentFullscreenMode) : FText::FromString("Select..."); })
				]
			]
		];

	// Spacer
	MenuContainer->AddSlot()
		.FillHeight(1.0f);

	// Apply Button
	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 10)
		[
			SNew(SRevoltMenuButton)
			.Text(LOCTEXT("Apply", "APPLY CHANGES"))
			.BaseColor(PrimaryColor)
			.OnClicked(this, &SRevoltMainMenu::OnApplySettingsClicked)
		];

	// Back Button
	MenuContainer->AddSlot()
		.AutoHeight()
		.Padding(0, 10)
		[
			SNew(SRevoltMenuButton)
			.Text(LOCTEXT("Back", "BACK"))
			.BaseColor(FLinearColor::White)
			.OnClicked(this, &SRevoltMainMenu::OnBackClicked)
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SRevoltMainMenu::OnResolutionChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
	CurrentResolution = NewItem;
}

void SRevoltMainMenu::OnFullscreenChanged(TSharedPtr<FString> NewItem, ESelectInfo::Type SelectInfo)
{
	CurrentFullscreenMode = NewItem;
}

FReply SRevoltMainMenu::OnApplySettingsClicked()
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		// Parse Resolution
		if (CurrentResolution.IsValid())
		{
			FString ResString = *CurrentResolution;
			FString WStr, HStr;
			if (ResString.Split(TEXT("x"), &WStr, &HStr))
			{
				int32 W = FCString::Atoi(*WStr);
				int32 H = FCString::Atoi(*HStr);
				Settings->SetScreenResolution(FIntPoint(W, H));
			}
		}

		// Parse Window Mode
		if (CurrentFullscreenMode.IsValid())
		{
			if (*CurrentFullscreenMode == "Fullscreen") Settings->SetFullscreenMode(EWindowMode::Fullscreen);
			else if (*CurrentFullscreenMode == "Windowed Fullscreen") Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			else Settings->SetFullscreenMode(EWindowMode::Windowed);
		}

		Settings->ApplySettings(false);
	}
	return FReply::Handled();
}

FReply SRevoltMainMenu::OnBackClicked()
{
	ConstructMainMenu();
	return FReply::Handled();
}


void SRevoltMenuButton::Construct(const FArguments& InArgs)
{
	OnClickedDelegate = InArgs._OnClicked;
	BaseColor = InArgs._BaseColor;
	CurrentScale = 1.0f;
	TargetScale = 1.0f;
	CurrentBrightness = 0.6f; // Initial dark background
	TargetBrightness = 0.6f;

	// 1. Create text block locally
	TSharedRef<STextBlock> LocalTextBlock = SNew(STextBlock)
		.Text(InArgs._Text)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
		.ColorAndOpacity(BaseColor)
		.ShadowOffset(FVector2D(1, 1))
		.ShadowColorAndOpacity(FLinearColor::Black);
	
	// Store references to member variables for animation
	TextBlock = LocalTextBlock;
	TextBlock->SetColorAndOpacity(BaseColor);

	// 2. Create inner layout
	TSharedRef<SWidget> InnerContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(6)
			[
				SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.ColorAndOpacity(BaseColor)
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(20, 0)
		[
			LocalTextBlock
		];

	// 3. Create border locally
	TSharedRef<SBorder> LocalBorder = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, CurrentBrightness))
		.Padding(FMargin(0))
		[
			InnerContent
		];

	// Store reference for animation
	BackgroundBorder = LocalBorder;

	// 4. Create Button
	TSharedRef<SButton> MyButton = SNew(SButton)
		.ContentPadding(FMargin(0))
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
		.Cursor(EMouseCursor::Hand)
		.OnClicked(FOnClicked::CreateSP(this, &SRevoltMenuButton::OnButtonClicked))
		.OnHovered(FSimpleDelegate::CreateSP(this, &SRevoltMenuButton::OnButtonHovered))
		.OnUnhovered(FSimpleDelegate::CreateSP(this, &SRevoltMenuButton::OnButtonUnhovered));

	// Manually attach content using local ref
	MyButton->SetContent(LocalBorder);

	// 5. Final Container
	TSharedRef<SBox> Container = SNew(SBox)
		.WidthOverride(400)
		.HeightOverride(60)
		.Padding(FMargin(0));
	
	Container->SetContent(MyButton);

	// Assign to this widget's ChildSlot
	this->ChildSlot
	[
		Container
	];
}

void SRevoltMenuButton::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Animate Scale
	CurrentScale = FMath::FInterpTo(CurrentScale, TargetScale, InDeltaTime, 15.0f);
	SetRenderTransform(FSlateRenderTransform(CurrentScale));

	// Animate Brightness
	CurrentBrightness = FMath::FInterpTo(CurrentBrightness, TargetBrightness, InDeltaTime, 10.0f);
	if (BackgroundBorder.IsValid())
	{
		BackgroundBorder->SetBorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, CurrentBrightness));
	}
}

FReply SRevoltMenuButton::OnButtonClicked()
{
	if (OnClickedDelegate.IsBound())
	{
		return OnClickedDelegate.Execute();
	}
	return FReply::Handled();
}

void SRevoltMenuButton::OnButtonHovered()
{
	TargetScale = 1.05f;
	TargetBrightness = 0.9f; // Brighter on hover
}

void SRevoltMenuButton::OnButtonUnhovered()
{
	TargetScale = 1.0f;
	TargetBrightness = 0.6f; // Back to dark
}

void SRevoltMainMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Fade in animation
	float Speed = (MenuConfig) ? MenuConfig->AnimationSpeed : 2.0f;
	if (MenuOpacity < 1.0f)
	{
		MenuOpacity += InDeltaTime * Speed;
		if (MenuOpacity > 1.0f) MenuOpacity = 1.0f;
	}
}

FLinearColor SRevoltMainMenu::GetMenuOpacity() const
{
	return FLinearColor(1, 1, 1, MenuOpacity);
}

FSlateColor SRevoltMainMenu::GetMenuOpacitySlate() const
{
	return FSlateColor(FLinearColor(1, 1, 1, MenuOpacity));
}

FReply SRevoltMainMenu::OnStartGameClicked()
{
	if (!MenuConfig || MenuConfig->StartGameLevel.IsNull() || !GEngine)
	{
		return FReply::Handled();
	}

	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}

	if (!World)
	{
		return FReply::Handled();
	}

	// In cooked builds, GetAssetName() doesn't work correctly for level loading.
	// Use the asset path to get the proper level name.
	FString LevelPath = MenuConfig->StartGameLevel.ToString();
	FString LevelName = FPackageName::ObjectPathToPackageName(LevelPath);

	UGameplayStatics::OpenLevel(World, FName(*LevelName));

	return FReply::Handled();
}

FReply SRevoltMainMenu::OnSettingsClicked()
{
	ConstructSettings();
	return FReply::Handled();
}

FReply SRevoltMainMenu::OnExitGameClicked()
{
	if (GEngine)
	{
		UWorld* World = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}

		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
			}
		}
	}
	return FReply::Handled();
}


void SRevoltMainMenu::ConstructLevelSelect()
{
	MenuContainer->ClearChildren();
	
	FLinearColor PrimaryColor = FLinearColor(1.0f, 0.2f, 0.4f, 1.0f);
	if (MenuConfig) PrimaryColor = MenuConfig->PrimaryColor;

	if (MenuContainer.IsValid())
	{
		MenuContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 0, 0, 30)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LevelSelectHeader", "SELECT LEVEL"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
				.ColorAndOpacity(FLinearColor::White)
			];

		// ScrollBox for levels
		TSharedPtr<SScrollBox> LevelList;
		MenuContainer->AddSlot()
			.FillHeight(1.0f)
			.Padding(0, 10)
			[
				SAssignNew(LevelList, SScrollBox)
			];

		if (MenuConfig)
		{
			for (const TSoftObjectPtr<UWorld>& LevelPtr : MenuConfig->SelectableLevels)
			{
				if (LevelPtr.IsNull()) continue;

				FString LevelName = LevelPtr.GetAssetName();
				// Strip prefix if any
				if (LevelName.StartsWith("L_") || LevelName.StartsWith("Map_"))
				{
					LevelName = LevelName.Mid(2); // Simple heuristic, can be improved
				}
				
				// Replace underscores with spaces for nicer display
				LevelName = LevelName.Replace(TEXT("_"), TEXT(" "));

				LevelList->AddSlot()
					.Padding(0, 5)
					[
						SNew(SRevoltMenuButton)
						.Text(FText::FromString(LevelName))
						.BaseColor(FLinearColor::White)
						.OnClicked(FOnClicked::CreateSP(this, &SRevoltMainMenu::OnSelectLevelClicked, LevelPtr))
					];
			}
		}

		if (LevelList->GetChildren()->Num() == 0)
		{
			LevelList->AddSlot()
				.Padding(0, 10)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoLevels", "No levels configured in Data Asset."))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
					.ColorAndOpacity(FLinearColor::Gray)
				];
		}

		// Back Button
		MenuContainer->AddSlot()
			.AutoHeight()
			.Padding(0, 10)
			[
				SNew(SRevoltMenuButton)
				.Text(LOCTEXT("Back", "BACK"))
				.BaseColor(FLinearColor::White)
				.OnClicked(this, &SRevoltMainMenu::OnBackClicked)
			];
	}
}

FReply SRevoltMainMenu::OnLoadLevelClicked()
{
	ConstructLevelSelect();
	return FReply::Handled();
}

FReply SRevoltMainMenu::OnSelectLevelClicked(TSoftObjectPtr<UWorld> LevelToLoad)
{
	if (!LevelToLoad.IsNull())
	{
		if (GEngine)
		{
			UWorld* World = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
				{
					World = Context.World();
					break;
				}
			}

			if (World)
			{
				// In cooked builds, GetAssetName() doesn't work correctly for level loading.
				// Use the asset path to get the proper level name.
				FString LevelPath = LevelToLoad.ToString();
				FString LevelName = FPackageName::ObjectPathToPackageName(LevelPath);
				UGameplayStatics::OpenLevel(World, FName(*LevelName));
			}
		}
	}
	return FReply::Handled();
}

FReply SRevoltMainMenu::OnWebsiteButtonClicked()
{
	if (!MenuConfig || !MenuConfig->bShowWebsiteButton || MenuConfig->WebsiteURL.IsEmpty())
	{
		return FReply::Handled();
	}

	// Open the URL in the default web browser
	FPlatformProcess::LaunchURL(*MenuConfig->WebsiteURL, nullptr, nullptr);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
