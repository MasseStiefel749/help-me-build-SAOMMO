// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainGameMode.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "VRCharacter.h"
#include "Engine/Engine.h"

AMainGameMode::AMainGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AMainPlayerController::StaticClass();
	DefaultCharacterClass = APlayerCharacter::StaticClass();
}

UClass* AMainGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Band 3 §20 point 5 (audit gaps G2 + G6): while an XR system (HMD) is active,
	// the player spawns as the Band 2 §7 VR pawn instead of the desktop character.
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		return AVRCharacter::StaticClass();
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
}
