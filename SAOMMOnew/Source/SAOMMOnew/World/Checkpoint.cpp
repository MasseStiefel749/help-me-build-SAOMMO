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

	// Slim landmark pole beside the volume (never on top of the spawn) plus
	// a floating diamond on top. Reads as a game marker, not architecture.
	BeaconMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconMesh"));
	BeaconMesh->SetupAttachment(CheckpointVolume);
	BeaconMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeaconTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeaconTop"));
	BeaconTop->SetupAttachment(CheckpointVolume);
	BeaconTop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BeaconObj(
		TEXT("/Engine/BasicShapes/Cube"));
	if (BeaconObj.Succeeded())
	{
		BeaconMesh->SetStaticMesh(BeaconObj.Object);
		BeaconMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 4.0f));
		BeaconMesh->SetRelativeLocation(FVector(250.0f, 0.0f, 200.0f));

		BeaconTop->SetStaticMesh(BeaconObj.Object);
		BeaconTop->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
		BeaconTop->SetRelativeLocation(FVector(250.0f, 0.0f, 460.0f));
		BeaconTop->SetRelativeRotation(FRotator(0.0f, 0.0f, 45.0f));
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
