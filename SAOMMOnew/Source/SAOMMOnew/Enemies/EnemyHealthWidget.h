// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthWidget.generated.h"

class UProgressBar;

/**
 *  Code-only enemy health bar (pushed from AEnemy, no tick, no assets).
 */
UCLASS(Blueprintable)
class UEnemyHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Sets the bar 0..1 (clamped). Safe before Construct. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetFraction(float InFraction);

protected:

	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<UProgressBar> Bar = nullptr;

	float PendingFraction = 1.0f;
};
