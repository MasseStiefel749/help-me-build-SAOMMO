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

	// Visible blade: stretched cube (blockout look). Without this combat is
	// invisible — the mesh asset normally comes from the (broken) Blueprint.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BladeMesh(
		TEXT("/Engine/BasicShapes/Cube"));
	if (BladeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(BladeMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 1.4f));
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	}
}

void ASword::BeginPlay()
{
	Super::BeginPlay();

	PreviousLocation = GetActorLocation();
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
