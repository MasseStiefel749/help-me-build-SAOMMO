// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOCheckpoint.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "SAOMMOCharacter.h"
#include "SAOMMOPlayerController.h"

ASAOCheckpoint::ASAOCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	CheckpointVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CheckpointVolume"));
	RootComponent = CheckpointVolume;
	CheckpointVolume->SetSphereRadius(CheckpointRadius);
	CheckpointVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CheckpointVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	CheckpointVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASAOCheckpoint::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	const ASAOMMOCharacter* Character = Cast<ASAOMMOCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	// The checkpoint transform faces along the checkpoint's forward axis so
	// respawns arrive looking into the region, not into a wall.
	FTransform SpawnTransform(GetActorRotation(), GetActorLocation(), FVector::OneVector);

	if (AController* Controller = Character->GetController())
	{
		if (ASAOMMOPlayerController* PC = Cast<ASAOMMOPlayerController>(Controller))
		{
			PC->SetRespawnTransform(SpawnTransform);
		}
	}
}
