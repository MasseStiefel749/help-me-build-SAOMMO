// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SAOMMOPlayerController.generated.h"

class UInputMappingContext;
class ASAOMMOCharacter;
class USAOMMOHudWidget;

/**
 *  SAOMMO player controller (target architecture).
 *
 *  Adds Enhanced Input mapping contexts and respawns the player character when
 *  it is destroyed (Band 3 §11). Kept deliberately smaller than the template
 *  controller; mobile touch controls are deferred.
 */
UCLASS()
class ASAOMMOPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input mapping contexts applied on possession. */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Character class to respawn when the possessed pawn is destroyed. */
	UPROPERTY(EditAnywhere, Category = "Respawn")
	TSubclassOf<ASAOMMOCharacter> CharacterClass;

	/** HUD widget class spawned for local players (defaults to the code HUD). */
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<USAOMMOHudWidget> HudWidgetClass;

	/** Live HUD instance (local players only). */
	UPROPERTY()
	TObjectPtr<USAOMMOHudWidget> HudWidget = nullptr;

	/** Transform used for respawns; can be updated to create checkpoints. */
	FTransform RespawnTransform;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

public:

	ASAOMMOPlayerController();

	/** Updates the respawn transform (e.g. from a checkpoint). */
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnTransform(const FTransform& NewRespawn);

protected:

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
};
