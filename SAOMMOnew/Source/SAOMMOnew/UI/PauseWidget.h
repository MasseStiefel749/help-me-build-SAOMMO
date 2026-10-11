// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UVerticalBox;

/**
 *  Code-only pause overlay (standard Esc behavior): full-screen dimmer,
 *  "PAUSE" title, Resume + Quit buttons. Same no-Editor-asset pattern as
 *  the HUD and death screen. Visual layout stays a manual check; state
 *  transitions are automation-tested via the owning controller.
 */
UCLASS(Blueprintable)
class UPauseWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Shows the overlay (idempotent). */
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void ShowPause();

	/** Hides the overlay (idempotent). */
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void HidePause();

	/** True while the overlay is in the viewport. */
	UFUNCTION(BlueprintCallable, Category = "Pause")
	bool IsPauseShown() const;

protected:

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnQuitClicked();

	/** Full-screen dimmer. */
	UPROPERTY()
	TObjectPtr<UBorder> Dimmer = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> ResumeButton = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> QuitButton = nullptr;
};
