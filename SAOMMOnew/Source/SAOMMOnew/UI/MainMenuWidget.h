// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UComboBoxString;
class UTextBlock;

/** Lightweight startup menu. Gameplay levels are opened only after Play. */
UCLASS()
class UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	void BuildLayout();
	void PopulateVideoOptions();
	void ApplyVideoOptions();

	UFUNCTION()
	void OnApplyClicked();

	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnQuitClicked();

	UPROPERTY()
	TObjectPtr<UComboBoxString> ResolutionCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> WindowModeCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> QualityCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> RenderScaleCombo = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> PlayButton = nullptr;

	bool bIsStartingGame = false;
};
