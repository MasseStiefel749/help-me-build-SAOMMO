// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

class UBorder;
class UTextBlock;
class UVerticalBox;

/**
 *  Code-only death overlay (Band 3 §13/§14: die -> restart reads as two
 *  distinct beats). Full-screen dark red dimmer, "YOU DIED" title and a
 *  "press any key" prompt - the respawn is player-driven (any key press)
 *  with the controller's fallback timer as safety net. Same no-Editor-
 *  asset pattern as the HUD.
 */
UCLASS(Blueprintable)
class UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Shows the overlay (safe to call again after a previous death). */
	UFUNCTION(BlueprintCallable, Category = "Death")
	void ShowDeath();

protected:

	virtual void NativeConstruct() override;

	/** Full-screen dark red dimmer. */
	UPROPERTY()
	TObjectPtr<UBorder> Dimmer = nullptr;

	/** Centered title block (title + respawn prompt). */
	UPROPERTY()
	TObjectPtr<UVerticalBox> CenterBox = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> RespawnText = nullptr;
};
