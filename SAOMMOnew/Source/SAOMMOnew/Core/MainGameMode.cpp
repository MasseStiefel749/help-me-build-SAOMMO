// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainGameMode.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"

AMainGameMode::AMainGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AMainPlayerController::StaticClass();
	DefaultCharacterClass = APlayerCharacter::StaticClass();
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
}
