// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOGameMode.h"
#include "SAOMMOCharacter.h"
#include "SAOMMOPlayerController.h"

ASAOMMOGameMode::ASAOMMOGameMode()
{
	DefaultPawnClass = ASAOMMOCharacter::StaticClass();
	PlayerControllerClass = ASAOMMOPlayerController::StaticClass();
	DefaultCharacterClass = ASAOMMOCharacter::StaticClass();
}

void ASAOMMOGameMode::BeginPlay()
{
	Super::BeginPlay();
}
