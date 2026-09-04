// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOSword.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ASAOSword::ASAOSword()
{
	PrimaryActorTick.bCanEverTick = true;

	BladeCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BladeCollision"));
	RootComponent = BladeCollision;
	BladeCollision->SetSphereRadius(15.0f);
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BladeCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASAOSword::BeginPlay()
{
	Super::BeginPlay();

	PreviousLocation = GetActorLocation();
}

void ASAOSword::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime > KINDA_SMALL_NUMBER)
	{
		const FVector CurrentLocation = GetActorLocation();
		SwingVelocity = (CurrentLocation - PreviousLocation) / DeltaTime;
		PreviousLocation = CurrentLocation;
	}
}
