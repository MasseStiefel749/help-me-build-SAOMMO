// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerHudWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "PlayerCharacter.h"
#include "ProgressionComponent.h"
#include "InteractionComponent.h"

void UPlayerHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBar)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HudCanvas"));
		WidgetTree->RootWidget = Canvas;

		// Stats block, top-left.
		UVerticalBox* Stats = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HudStats"));
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
		LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
		FocusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FocusText"));
		Stats->AddChildToVerticalBox(HealthBar);
		Stats->AddChildToVerticalBox(HealthText);
		Stats->AddChildToVerticalBox(LevelText);
		Stats->AddChildToVerticalBox(FocusText);

		// Center crosshair for aiming swings and E-interactions.
		Crosshair = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Crosshair"));
		Crosshair->SetText(FText::FromString(TEXT("+")));
		Crosshair->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.8f));
		Crosshair->SetJustification(ETextJustify::Center);

		if (Canvas)
		{
			if (UCanvasPanelSlot* StatsSlot = Canvas->AddChildToCanvas(Stats))
			{
				StatsSlot->SetAnchors(FAnchors(0.0f, 0.0f));
				StatsSlot->SetOffsets(FMargin(20.0f, 20.0f, 320.0f, 140.0f));
			}
			if (UCanvasPanelSlot* CrossSlot = Canvas->AddChildToCanvas(Crosshair))
			{
				CrossSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				CrossSlot->SetOffsets(FMargin(-12.0f, -16.0f, 24.0f, 32.0f));
				CrossSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			}
		}
	}

	SetStats(1.0f, 1.0f, 1, 0.0f, FString());
}

void UPlayerHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Poll the possessed SAOMMO character; HUD stays valid across
	// death/respawn because it re-resolves the pawn every tick.
	APawn* Pawn = GetOwningPlayerPawn();
	const APlayerCharacter* Character = Cast<APlayerCharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	float MaxHealth = 1.0f;
	float CurrentHealth = 0.0f;
	int32 Level = 1;
	float Experience = 0.0f;
	FString FocusName;

	// Health via TakeDamage-clamped values exposed on the character.
	// (UProperties are private-readable; use the HUD update with polling
	// through the public getters below if extended.)
	CurrentHealth = Character->GetHealth();
	MaxHealth = Character->GetMaxHealth();

	if (const UProgressionComponent* Prog = Character->GetProgression())
	{
		Level = Prog->Level;
		Experience = Prog->Experience;
	}
	if (const UInteractionComponent* Interaction = Character->GetInteraction())
	{
		if (AActor* Focused = Interaction->GetFocusedActor())
		{
			// GetActorLabel() is editor-only; GetName() works in game builds.
			FocusName = Focused->GetName();
		}
	}

	SetStats(CurrentHealth, MaxHealth, Level, Experience, FocusName);
}

void UPlayerHudWidget::SetStats(float CurrentHealth, float MaxHealth, int32 Level, float Experience, const FString& FocusName)
{
	const float Denom = FMath::Max(1.0f, MaxHealth);
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(CurrentHealth / Denom, 0.0f, 1.0f));
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), CurrentHealth, MaxHealth)));
	}
	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv %d  XP %.0f"), Level, Experience)));
	}
	if (FocusText)
	{
		FocusText->SetText(FText::FromString(FocusName.IsEmpty() ? FString() : FString::Printf(TEXT("[E] %s"), *FocusName)));
	}
}
