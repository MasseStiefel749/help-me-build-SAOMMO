// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SAOMMOGameMode.generated.h"

class ASAOMMOCharacter;

/**
 *  SAOMMO game mode (target architecture).
 *
 *  Owns the default player pawn (ASAOMMOCharacter) and the player controller.
 *  Replaces the template Variant_Combat game mode as the project migrates to
 *  the Band 2 architecture.
 */
UCLASS()
class ASAOMMOGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ASAOMMOGameMode();

	/** Character class spawned for players. Assign a Blueprint subclass in the editor. */
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASAOMMOCharacter> DefaultCharacterClass;

protected:

	virtual void BeginPlay() override;
};
