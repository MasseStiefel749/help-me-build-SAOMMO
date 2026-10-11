// Copyright Epic Games, Inc. All Rights Reserved.

#include "PauseWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"

void UPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PauseCanvas"));
		WidgetTree->RootWidget = Canvas;

		Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PauseDimmer"));
		Dimmer->SetBrushColor(FLinearColor(0.01f, 0.02f, 0.04f, 0.75f));
		if (UCanvasPanelSlot* DimmerSlot = Canvas->AddChildToCanvas(Dimmer))
		{
			DimmerSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			DimmerSlot->SetOffsets(FMargin(0.0f));
		}

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseLayout"));
		Dimmer->SetContent(Layout);

		auto MakeText = [this](const FString& Text, int32 Size, const FLinearColor& Color, bool bBold = false)
		{
			UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Block->SetText(FText::FromString(Text));
			Block->SetColorAndOpacity(Color);
			Block->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), Size));
			Block->SetJustification(ETextJustify::Center);
			return Block;
		};

		auto AddLine = [Layout](UWidget* Child, const FMargin& LinePadding = FMargin(0.0f, 6.0f))
		{
			if (UVerticalBoxSlot* Slot = Layout->AddChildToVerticalBox(Child))
			{
				Slot->SetPadding(LinePadding);
				Slot->SetHorizontalAlignment(HAlign_Center);
			}
		};

		AddLine(MakeText(TEXT("PAUSE"), 48, FLinearColor(0.92f, 0.93f, 0.95f), true), FMargin(0.0f, 120.0f, 0.0f, 6.0f));
		AddLine(MakeText(TEXT("Esc – Fortsetzen"), 16, FLinearColor(0.65f, 0.72f, 0.78f)));

		auto MakeButton = [this, &MakeText](const FString& Label)
		{
			UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
			Button->SetBackgroundColor(FLinearColor(0.15f, 0.22f, 0.30f));
			Button->SetContent(MakeText(Label, 18, FLinearColor::White, true));
			return Button;
		};

		ResumeButton = MakeButton(TEXT("FORTSETZEN"));
		ResumeButton->OnClicked.AddDynamic(this, &UPauseWidget::OnResumeClicked);
		AddLine(ResumeButton, FMargin(0.0f, 24.0f, 0.0f, 6.0f));

		QuitButton = MakeButton(TEXT("BEENDEN"));
		QuitButton->OnClicked.AddDynamic(this, &UPauseWidget::OnQuitClicked);
		AddLine(QuitButton);
	}
}

void UPauseWidget::ShowPause()
{
	if (!IsInViewport())
	{
		AddToViewport(400);
	}
	SetVisibility(ESlateVisibility::Visible);
}

void UPauseWidget::HidePause()
{
	RemoveFromParent();
}

bool UPauseWidget::IsPauseShown() const
{
	return IsInViewport();
}

void UPauseWidget::OnResumeClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
	}
	HidePause();
}

void UPauseWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
