// Copyright Epic Games, Inc. All Rights Reserved.

#include "Checkpoint.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
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

	// Visible landmark pillar (was fully invisible before). Offset to the
	// side so a checkpoint placed exactly on a spawn point does not swallow
	// the player, and slimmed so it marks without blocking the view.
	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(CheckpointVolume);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BeaconObj(
		TEXT("/Engine/BasicShapes/Cube"));
	if (BeaconObj.Succeeded())
	{
		BeaconMesh->SetStaticMesh(BeaconObj.Object);
		BeaconMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 4.0f));
		BeaconMesh->SetRelativeLocation(FVector(250.0f, 0.0f, 200.0f));
	}
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
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Checkpoint reached"));
			}
		}
	}
}
