// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeathScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

void UDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Dimmer)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DeathCanvas"));
		WidgetTree->RootWidget = Canvas;

		// Full-screen dark red dimmer (readable under any level lighting).
		Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dimmer"));
		Dimmer->SetBrushColor(FLinearColor(0.09f, 0.0f, 0.0f, 0.88f));

		// Centered title + respawn prompt.
		CenterBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DeathCenter"));
		TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathTitle"));
		TitleText->SetText(FText::FromString(TEXT("YOU DIED")));
		TitleText->SetColorAndOpacity(FLinearColor(0.85f, 0.08f, 0.08f, 1.0f));
		TitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 72));
		TitleText->SetJustification(ETextJustify::Center);
		RespawnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DeathRespawn"));
		RespawnText->SetText(FText::FromString(TEXT("PRESS ANY KEY TO RESPAWN")));
		RespawnText->SetColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.92f, 1.0f));
		RespawnText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 24));
		RespawnText->SetJustification(ETextJustify::Center);

		CenterBox->AddChildToVerticalBox(TitleText);
		CenterBox->AddChildToVerticalBox(RespawnText);

		if (Canvas)
		{
			// Dimmer fills the screen.
			if (UCanvasPanelSlot* DimSlot = Canvas->AddChildToCanvas(Dimmer))
			{
				DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				DimSlot->SetOffsets(FMargin(0.0f));
			}
			// Center box sits in the middle, sized to its content.
			if (UCanvasPanelSlot* BoxSlot = Canvas->AddChildToCanvas(CenterBox))
			{
				BoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				BoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				BoxSlot->SetOffsets(FMargin(0.0f));
			}
		}
	}
}

void UDeathScreenWidget::ShowDeath()
{
	if (RespawnText)
	{
		RespawnText->SetText(FText::FromString(TEXT("PRESS ANY KEY TO RESPAWN")));
	}
}
