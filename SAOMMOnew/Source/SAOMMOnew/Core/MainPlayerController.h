// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class APlayerCharacter;
class UPlayerHudWidget;

/**
 *  SAOMMO player controller (target architecture).
 *
 *  Adds Enhanced Input mapping contexts and respawns the player character when
 *  it is destroyed (Band 3 §11). Kept deliberately smaller than the template
 *  controller; mobile touch controls are deferred.
 */
UCLASS()
class AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input mapping contexts applied on possession. */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction = nullptr;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction = nullptr;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction = nullptr;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ToggleCameraAction = nullptr;

	/** Fallback input actions (FObjectFinder defaults; Blueprint may override). */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction = nullptr;

	/** Transient fallback mapping, built when no contexts are assigned. */
	UPROPERTY()
	TObjectPtr<UInputMappingContext> FallbackMapping = nullptr;

	/** Character class to respawn when the possessed pawn is destroyed. */
	UPROPERTY(EditAnywhere, Category = "Respawn")
	TSubclassOf<APlayerCharacter> CharacterClass;

	/** HUD widget class spawned for local players (defaults to the code HUD). */
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UPlayerHudWidget> HudWidgetClass;

	/** Live HUD instance (local players only). */
	UPROPERTY()
	TObjectPtr<UPlayerHudWidget> HudWidget = nullptr;

	/** Transform used for respawns; can be updated to create checkpoints. */
	FTransform RespawnTransform;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

public:

	AMainPlayerController();

	/** Updates the respawn transform (e.g. from a checkpoint). */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnTransform(const FTransform& NewRespawn);

	/** Returns the current respawn transform. */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	FTransform GetRespawnTransform() const { return RespawnTransform; }

	/** Saves pawn transform, health, inventory and progression to the slot. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveProgress();

	/** Loads the slot into the current pawn (teleport + state). False if no save. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadProgress();

	/** Save slot name (shared with UPlayerSaveGame default). */
	UPROPERTY(EditAnywhere, Category = "Save")
	FString SaveSlotName = TEXT("PlayerSave");

protected:

	/** Builds the transient fallback mapping (WASD/mouse/Space/LMB/V/E). */
	void BuildFallbackMapping();

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
};
