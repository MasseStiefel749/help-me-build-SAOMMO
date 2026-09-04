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

	GemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GemMesh"));
	GemMesh->SetupAttachment(BladeCollision);
	GemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Real sword (CC0 FantasySword, Content/Weapons/FantasySword): parts are
	// modeled along Y with the tip at -Y (pommel gem +14.6, guard -19.2).
	// Stand them blade-up with a ROLL (rotation about X): note Pitch rotates
	// about Y and would leave Y-extents lying flat (the sideways-sword bug).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RealBlade(
		TEXT("/Game/Weapons/FantasySword/Blade"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RealGuard(
		TEXT("/Game/Weapons/FantasySword/CrossG"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RealHandle(
		TEXT("/Game/Weapons/FantasySword/Hilt"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RealGem(
		TEXT("/Game/Weapons/FantasySword/Gem"));
	if (RealBlade.Succeeded())
	{
		const FRotator Upright(0.0f, 0.0f, -90.0f);
		Mesh->SetStaticMesh(RealBlade.Object);
		Mesh->SetRelativeLocation(FVector::ZeroVector);
		Mesh->SetRelativeRotation(Upright);
		Mesh->SetRelativeScale3D(FVector::OneVector);
		if (RealGuard.Succeeded() && GuardMesh)
		{
			GuardMesh->SetStaticMesh(RealGuard.Object);
			GuardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 19.2f));
			GuardMesh->SetRelativeRotation(Upright);
			GuardMesh->SetRelativeScale3D(FVector::OneVector);
		}
		if (RealHandle.Succeeded() && HandleMesh)
		{
			HandleMesh->SetStaticMesh(RealHandle.Object);
			HandleMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.9f));
			HandleMesh->SetRelativeRotation(Upright);
			HandleMesh->SetRelativeScale3D(FVector::OneVector);
		}
		if (RealGem.Succeeded() && GemMesh)
		{
			GemMesh->SetStaticMesh(RealGem.Object);
			GemMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -14.6f));
			GemMesh->SetRelativeRotation(Upright);
			GemMesh->SetRelativeScale3D(FVector::OneVector);
		}
		return;
	}

	// Fallback blockout sword built from boxes (no sword asset in project).
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
	if (GemMesh)
	{
		GemMesh->SetCastShadow(bEnabled);
		GemMesh->bCastHiddenShadow = bEnabled;
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
