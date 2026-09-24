// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

class APlayerCharacter;

/**
 *  SAOMMO game mode (target architecture).
 *
 *  Owns the default player pawn (APlayerCharacter) and the player controller.
 *  Replaces the template Variant_Combat game mode as the project migrates to
 *  the Band 2 architecture.
 */
UCLASS()
class AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AMainGameMode();

	/** Character class spawned for players. Assign a Blueprint subclass in the editor. */
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<APlayerCharacter> DefaultCharacterClass;

protected:

	virtual void BeginPlay() override;

	/**
	 *  Band 3 §20 point 5 (audit gaps G2 + G6): while an XR system (HMD) is
	 *  active, players spawn as the Band 2 §7 VR pawn; otherwise the desktop
	 *  character (default pawn class) is used unchanged.
	 */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
