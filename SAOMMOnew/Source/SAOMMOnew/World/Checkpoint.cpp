// Copyright Epic Games, Inc. All Rights Reserved.

#include "Checkpoint.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"

ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	CheckpointVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CheckpointVolume"));
	RootComponent = CheckpointVolume;
	CheckpointVolume->SetSphereRadius(CheckpointRadius);
	CheckpointVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CheckpointVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	CheckpointVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ACheckpoint::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	const APlayerCharacter* Character = Cast<APlayerCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	// The checkpoint transform faces along the checkpoint's forward axis so
	// respawns arrive looking into the region, not into a wall.
	FTransform SpawnTransform(GetActorRotation(), GetActorLocation(), FVector::OneVector);

	if (AController* Controller = Character->GetController())
	{
		if (AMainPlayerController* PC = Cast<AMainPlayerController>(Controller))
		{
			PC->SetRespawnTransform(SpawnTransform);
		}
	}
}
