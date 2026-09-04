// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHudWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UVerticalBox;

/**
 *  Code-only HUD (Band 3 playable feedback, no Editor widget needed).
 *
 *  Polls the owning pawn every tick: health bar, level/XP line and the
 *  current interaction focus name. Created by AMainPlayerController for
 *  local players; safe with no pawn (shows empty defaults).
 */
UCLASS(Blueprintable)
class UPlayerHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Pushes fresh stats into the bars/texts. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetStats(float CurrentHealth, float MaxHealth, int32 Level, float Experience, const FString& FocusName);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> HealthText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> FocusText = nullptr;
};
