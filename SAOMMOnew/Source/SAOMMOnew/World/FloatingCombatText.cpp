// Copyright Epic Games, Inc. All Rights Reserved.

#include "FloatingCombatText.h"
#include "Components/TextRenderComponent.h"

AFloatingCombatText::AFloatingCombatText()
{
	PrimaryActorTick.bCanEverTick = true;

	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	RootComponent = TextComp;
	TextComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TextComp->SetWorldSize(36.0f);
	TextComp->SetHorizontalAlignment(EHTA_Center);
	TextComp->SetVerticalAlignment(EVRTA_TextCenter);
}

void AFloatingCombatText::Configure(float Amount, FLinearColor Color)
{
	if (TextComp)
	{
		TextComp->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Amount)));
		TextComp->SetTextRenderColor(Color.ToFColor(false));
	}
	SetLifeSpan(LifeTime);
}

void AFloatingCombatText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, RiseSpeed * DeltaTime));
}
