// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"

namespace
{
	// Bright sky, warm stone, and leaf green echo the landscape references while
	// keeping the menu readable even when the startup map contains no world art.
	const FLinearColor MenuBackground(0.12f, 0.30f, 0.50f, 1.0f);
	const FLinearColor PanelBackground(0.88f, 0.84f, 0.70f, 1.0f);
	const FLinearColor AccentColor(0.78f, 0.43f, 0.13f, 1.0f);
	const FLinearColor PrimaryButtonColor(0.06f, 0.34f, 0.23f, 1.0f);
	const FLinearColor PrimaryTextColor(0.075f, 0.12f, 0.16f, 1.0f);
	const FLinearColor SecondaryTextColor(0.22f, 0.27f, 0.29f, 1.0f);

	FString ResolutionToString(const FIntPoint& Resolution)
	{
		return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Display, TEXT("SAOMMO menu: NativeConstruct started; WidgetTree=%s"), WidgetTree ? TEXT("valid") : TEXT("null"));
	SetIsFocusable(true);
	BuildLayout();
	PopulateVideoOptions();
	UE_LOG(LogTemp, Display, TEXT("SAOMMO menu: layout built; root=%s"), WidgetTree && WidgetTree->RootWidget ? *WidgetTree->RootWidget->GetName() : TEXT("none"));
}

void UMainMenuWidget::BuildLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	auto MakeText = [this](const FString& Text, int32 Size, const FLinearColor& Color, bool bBold = false)
	{
		UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(Color);
		Block->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), Size));
		return Block;
	};

	auto MakeButton = [this, &MakeText](const FString& Label, const FLinearColor& Color, int32 FontSize)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		Button->SetBackgroundColor(Color);
		Button->SetColorAndOpacity(FLinearColor::White);
		Button->SetContent(MakeText(Label, FontSize, FLinearColor::White, true));
		return Button;
	};

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MainMenuCanvas"));
	WidgetTree->RootWidget = Canvas;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MainMenuBackground"));
	Background->SetBrushColor(MenuBackground);
	if (UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MainMenuCard"));
	Card->SetBrushColor(PanelBackground);
	Card->SetPadding(FMargin(48.0f, 36.0f));
	if (UCanvasPanelSlot* CardSlot = Canvas->AddChildToCanvas(Card))
	{
		CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CardSlot->SetOffsets(FMargin(-430.0f, -330.0f, 860.0f, 660.0f));
	}

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainMenuLayout"));
	Card->SetContent(Layout);

	auto AddContent = [Layout](UWidget* Child, const FMargin& ContentPadding = FMargin(0.0f))
	{
		if (UVerticalBoxSlot* Slot = Layout->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(ContentPadding);
		}
	};

	UTextBlock* Eyebrow = MakeText(TEXT("DAS ABENTEUER BEGINNT"), 14, AccentColor, true);
	AddContent(Eyebrow, FMargin(0.0f, 0.0f, 0.0f, 5.0f));

	UTextBlock* Title = MakeText(TEXT("SAOMMO"), 54, PrimaryTextColor, true);
	AddContent(Title, FMargin(0.0f, 0.0f, 0.0f, 3.0f));

	UTextBlock* Subtitle = MakeText(TEXT("Deine Welt wartet."), 18, SecondaryTextColor);
	AddContent(Subtitle, FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	UBorder* Divider = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuDivider"));
	Divider->SetBrushColor(AccentColor);
	USizeBox* DividerSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MenuDividerSize"));
	DividerSize->SetHeightOverride(2.0f);
	DividerSize->SetContent(Divider);
	AddContent(DividerSize, FMargin(0.0f, 0.0f, 0.0f, 16.0f));

	AddContent(MakeText(TEXT("GRAFIK & AUFLÖSUNG"), 15, AccentColor, true), FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	auto AddSettingRow = [this, Layout, &MakeText](const FString& Label, const FString& WidgetName, TObjectPtr<UComboBoxString>& OutCombo)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTextBlock* LabelText = MakeText(Label, 16, PrimaryTextColor);
		if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText))
		{
			LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 20.0f, 0.0f));
		}

		OutCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), *WidgetName);
		if (UHorizontalBoxSlot* ComboSlot = Row->AddChildToHorizontalBox(OutCombo))
		{
			ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		if (UVerticalBoxSlot* RowSlot = Layout->AddChildToVerticalBox(Row))
		{
			RowSlot->SetPadding(FMargin(0.0f, 6.0f));
		}
	};

	AddSettingRow(TEXT("Auflösung"), TEXT("ResolutionCombo"), ResolutionCombo);
	AddSettingRow(TEXT("Anzeigemodus"), TEXT("WindowModeCombo"), WindowModeCombo);
	AddSettingRow(TEXT("Grafikqualität"), TEXT("QualityCombo"), QualityCombo);
	AddSettingRow(TEXT("Render-Skalierung"), TEXT("RenderScaleCombo"), RenderScaleCombo);

	UHorizontalBox* ActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SettingsActions"));
	UButton* ApplyButton = MakeButton(TEXT("EINSTELLUNGEN ANWENDEN"), FLinearColor(0.20f, 0.27f, 0.30f, 1.0f), 14);
	ApplyButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnApplyClicked);
	if (UHorizontalBoxSlot* ApplySlot = ActionRow->AddChildToHorizontalBox(ApplyButton))
	{
		ApplySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	AddContent(ActionRow, FMargin(0.0f, 10.0f, 0.0f, 4.0f));

	StatusText = MakeText(TEXT("Einstellungen werden lokal gespeichert."), 12, SecondaryTextColor);
	AddContent(StatusText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UHorizontalBox* MainActions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainMenuActions"));
	PlayButton = MakeButton(TEXT("SPIELEN"), PrimaryButtonColor, 17);
	PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	if (UHorizontalBoxSlot* PlaySlot = MainActions->AddChildToHorizontalBox(PlayButton))
	{
		PlaySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		PlaySlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
	}

	UButton* QuitButton = MakeButton(TEXT("BEENDEN"), FLinearColor(0.37f, 0.16f, 0.12f, 1.0f), 14);
	QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	if (UHorizontalBoxSlot* QuitSlot = MainActions->AddChildToHorizontalBox(QuitButton))
	{
		QuitSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	AddContent(MainActions);
}

void UMainMenuWidget::PopulateVideoOptions()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings || !ResolutionCombo || !WindowModeCombo || !QualityCombo || !RenderScaleCombo)
	{
		return;
	}

	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	const FIntPoint CurrentResolution = Settings->GetScreenResolution();
	if (Resolutions.IsEmpty())
	{
		Resolutions.Add(Settings->GetDesktopResolution());
	}
	if (!Resolutions.Contains(CurrentResolution))
	{
		Resolutions.Add(CurrentResolution);
	}
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.Y == B.Y ? A.X < B.X : A.Y < B.Y;
	});

	ResolutionCombo->ClearOptions();
	for (const FIntPoint& Resolution : Resolutions)
	{
		if (Resolution.X >= 800 && Resolution.Y >= 600)
		{
			ResolutionCombo->AddOption(ResolutionToString(Resolution));
		}
	}
	ResolutionCombo->SetSelectedOption(ResolutionToString(CurrentResolution));

	WindowModeCombo->AddOption(TEXT("Fenster"));
	WindowModeCombo->AddOption(TEXT("Randloses Vollbild"));
	WindowModeCombo->AddOption(TEXT("Exklusives Vollbild"));
	switch (Settings->GetFullscreenMode())
	{
	case EWindowMode::Windowed:
		WindowModeCombo->SetSelectedOption(TEXT("Fenster"));
		break;
	case EWindowMode::Fullscreen:
		WindowModeCombo->SetSelectedOption(TEXT("Exklusives Vollbild"));
		break;
	default:
		WindowModeCombo->SetSelectedOption(TEXT("Randloses Vollbild"));
		break;
	}

	const TCHAR* QualityNames[] = { TEXT("Niedrig"), TEXT("Mittel"), TEXT("Hoch"), TEXT("Episch") };
	for (const TCHAR* Name : QualityNames)
	{
		QualityCombo->AddOption(Name);
	}
	QualityCombo->SetSelectedIndex(FMath::Clamp(Settings->GetOverallScalabilityLevel(), 0, 3));

	RenderScaleCombo->AddOption(TEXT("60 %"));
	RenderScaleCombo->AddOption(TEXT("75 %"));
	RenderScaleCombo->AddOption(TEXT("85 %"));
	RenderScaleCombo->AddOption(TEXT("100 %"));
	float CurrentScale = 100.0f;
	float Normalized = 1.0f;
	float MinScale = 50.0f;
	float MaxScale = 100.0f;
	Settings->GetResolutionScaleInformationEx(Normalized, CurrentScale, MinScale, MaxScale);
	const int32 ScaleIndex = CurrentScale < 67.5f ? 0 : CurrentScale < 80.0f ? 1 : CurrentScale < 92.5f ? 2 : 3;
	RenderScaleCombo->SetSelectedIndex(ScaleIndex);
}

void UMainMenuWidget::ApplyVideoOptions()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		return;
	}

	FString WidthText;
	FString HeightText;
	if (ResolutionCombo && ResolutionCombo->GetSelectedOption().Split(TEXT(" x "), &WidthText, &HeightText))
	{
		const FIntPoint Resolution(FCString::Atoi(*WidthText), FCString::Atoi(*HeightText));
		if (Resolution.X >= 800 && Resolution.Y >= 600)
		{
			Settings->SetScreenResolution(Resolution);
		}
	}

	if (WindowModeCombo)
	{
		const FString Mode = WindowModeCombo->GetSelectedOption();
		if (Mode == TEXT("Fenster"))
		{
			Settings->SetFullscreenMode(EWindowMode::Windowed);
		}
		else if (Mode == TEXT("Exklusives Vollbild"))
		{
			Settings->SetFullscreenMode(EWindowMode::Fullscreen);
		}
		else
		{
			Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		}
	}

	if (QualityCombo)
	{
		Settings->SetOverallScalabilityLevel(FMath::Clamp(QualityCombo->GetSelectedIndex(), 0, 3));
	}

	if (RenderScaleCombo)
	{
		static const float Scales[] = { 60.0f, 75.0f, 85.0f, 100.0f };
		const int32 Index = FMath::Clamp(RenderScaleCombo->GetSelectedIndex(), 0, static_cast<int32>(UE_ARRAY_COUNT(Scales)) - 1);
		Settings->SetResolutionScaleValueEx(Scales[Index]);
	}

	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

void UMainMenuWidget::OnApplyClicked()
{
	ApplyVideoOptions();
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Einstellungen gespeichert.")));
	}
}

void UMainMenuWidget::OnPlayClicked()
{
	if (bIsStartingGame || !GetWorld())
	{
		return;
	}

	ApplyVideoOptions();
	bIsStartingGame = true;
	if (PlayButton)
	{
		PlayButton->SetIsEnabled(false);
		if (UTextBlock* Label = Cast<UTextBlock>(PlayButton->GetContent()))
		{
			Label->SetText(FText::FromString(TEXT("WELT WIRD GELADEN ...")));
		}
	}
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Starting Reach wird geladen ...")));
	}

	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Levels/StartingReach/L_StartingReach")));
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
