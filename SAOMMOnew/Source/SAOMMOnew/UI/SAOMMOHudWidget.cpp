// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOHudWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "SAOMMOCharacter.h"
#include "SAOMMOProgressionComponent.h"
#include "SAOMMOInteractionComponent.h"

void USAOMMOHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBar)
	{
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HudRoot"));
		WidgetTree->RootWidget = Root;

		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
		LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
		FocusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FocusText"));

		if (Root)
		{
			Root->AddChildToVerticalBox(HealthBar);
			Root->AddChildToVerticalBox(HealthText);
			Root->AddChildToVerticalBox(LevelText);
			Root->AddChildToVerticalBox(FocusText);
		}
	}

	SetStats(1.0f, 1.0f, 1, 0.0f, FString());
}

void USAOMMOHudWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Poll the possessed SAOMMO character; HUD stays valid across
	// death/respawn because it re-resolves the pawn every tick.
	APawn* Pawn = GetOwningPlayerPawn();
	const ASAOMMOCharacter* Character = Cast<ASAOMMOCharacter>(Pawn);
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

	if (const USAOMMOProgressionComponent* Prog = Character->GetProgression())
	{
		Level = Prog->Level;
		Experience = Prog->Experience;
	}
	if (const USAOMMOInteractionComponent* Interaction = Character->GetInteraction())
	{
		if (AActor* Focused = Interaction->GetFocusedActor())
		{
			// GetActorLabel() is editor-only; GetName() works in game builds.
			FocusName = Focused->GetName();
		}
	}

	SetStats(CurrentHealth, MaxHealth, Level, Experience, FocusName);
}

void USAOMMOHudWidget::SetStats(float CurrentHealth, float MaxHealth, int32 Level, float Experience, const FString& FocusName)
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
