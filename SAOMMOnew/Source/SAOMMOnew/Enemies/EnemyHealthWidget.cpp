// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyHealthWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"

void UEnemyHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Bar)
	{
		Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EnemyBar"));
		Bar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.15f, 0.15f));
		WidgetTree->RootWidget = Bar;
	}

	Bar->SetPercent(FMath::Clamp(PendingFraction, 0.0f, 1.0f));
}

void UEnemyHealthWidget::SetFraction(float InFraction)
{
	PendingFraction = FMath::Clamp(InFraction, 0.0f, 1.0f);
	if (Bar)
	{
		Bar->SetPercent(PendingFraction);
	}
}
