// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingCombatText.generated.h"

class UTextRenderComponent;

/**
 *  Floating damage number (SAO-style hit feedback).
 *
 *  Spawned by the combat sweep on every damaging hit: rises for a second,
 *  then destroys itself. Pure code + engine default font, no assets.
 */
UCLASS(Blueprintable)
class AFloatingCombatText : public AActor
{
	GENERATED_BODY()

public:

	AFloatingCombatText();

	/** Sets the displayed number and color. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Configure(float Amount, FLinearColor Color);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* TextComp;

	/** Seconds the number stays alive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0.1, Units = "s"))
	float LifeTime = 1.0f;

	/** Rise speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (Units = "cm/s"))
	float RiseSpeed = 60.0f;

	float Age = 0.0f;

	virtual void Tick(float DeltaTime) override;
};
