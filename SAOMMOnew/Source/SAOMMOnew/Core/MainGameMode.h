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
};
