// Copyright Epic Games, Inc. All Rights Reserved.

#include "Sword.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ASword::ASword()
{
	PrimaryActorTick.bCanEverTick = true;

	BladeCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BladeCollision"));
	RootComponent = BladeCollision;
	BladeCollision->SetSphereRadius(15.0f);
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BladeCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GuardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuardMesh"));
	GuardMesh->SetupAttachment(BladeCollision);
	GuardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	HandleMesh->SetupAttachment(BladeCollision);
	HandleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Blockout sword built from boxes (no sword asset in the project): long
	// thin blade, wide flat guard, short grip. Reads as a sword instead of
	// a stick; a real mesh/material replaces this one-for-one later.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMesh(
		TEXT("/Engine/BasicShapes/Cube"));
	if (BoxMesh.Succeeded())
	{
		Mesh->SetStaticMesh(BoxMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 1.4f));
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));

		GuardMesh->SetStaticMesh(BoxMesh.Object);
		GuardMesh->SetRelativeScale3D(FVector(0.28f, 0.1f, 0.06f));
		GuardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));

		HandleMesh->SetStaticMesh(BoxMesh.Object);
		HandleMesh->SetRelativeScale3D(FVector(0.07f, 0.07f, 0.3f));
		HandleMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -25.0f));
	}
}

void ASword::BeginPlay()
{
	Super::BeginPlay();

	PreviousLocation = GetActorLocation();
}

void ASword::SetShadowCasting(bool bEnabled)
{
	if (Mesh)
	{
		Mesh->SetCastShadow(bEnabled);
		Mesh->bCastHiddenShadow = bEnabled;
	}
	if (GuardMesh)
	{
		GuardMesh->SetCastShadow(bEnabled);
		GuardMesh->bCastHiddenShadow = bEnabled;
	}
	if (HandleMesh)
	{
		HandleMesh->SetCastShadow(bEnabled);
		HandleMesh->bCastHiddenShadow = bEnabled;
	}
}

void ASword::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime > KINDA_SMALL_NUMBER)
	{
		const FVector CurrentLocation = GetActorLocation();
		SwingVelocity = (CurrentLocation - PreviousLocation) / DeltaTime;
		PreviousLocation = CurrentLocation;
	}
}
