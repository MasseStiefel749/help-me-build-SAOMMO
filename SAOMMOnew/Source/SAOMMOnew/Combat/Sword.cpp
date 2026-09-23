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
	TipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TipMesh"));
	TipMesh->SetupAttachment(BladeCollision);
	TipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Preferred hero: the authored one-mesh sword (source:
	// ArtSource/Sword/make_sword.py, imported to /Game/Weapons). Origin at
	// grip center, tip along local +Z (matches the aim code's up-vector
	// convention), so identity assembly is exact.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SAOSwordObj(
		TEXT("/Game/Weapons/SM_SAOSword"));
	if (SAOSwordObj.Succeeded())
	{
		Mesh->SetStaticMesh(SAOSwordObj.Object);
		Mesh->SetRelativeLocation(FVector::ZeroVector);
		Mesh->SetRelativeRotation(FRotator::ZeroRotator);
		Mesh->SetRelativeScale3D(FVector::OneVector);
		return;
	}

	// Fallback hero: Atlas BlackSword parts share one origin and are
	// already vertical (tip +Z, pommel -Z, grip at 0), so identity assembly
	// is exact - no rotations, no offsets, nothing to get wrong.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroBlade(
		TEXT("/Game/Atlas/Weapons/BlackSword/Blade"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroGuard(
		TEXT("/Game/Atlas/Weapons/BlackSword/Guard"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroHandle(
		TEXT("/Game/Atlas/Weapons/BlackSword/Grip"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroTip(
		TEXT("/Game/Atlas/Weapons/BlackSword/Tip"));
	if (HeroBlade.Succeeded())
	{
		const FRotator HangDown(-90.0f, 0.0f, 0.0f);
		Mesh->SetStaticMesh(HeroBlade.Object);
		Mesh->SetRelativeLocation(FVector::ZeroVector);
		Mesh->SetRelativeRotation(HangDown);
		Mesh->SetRelativeScale3D(FVector::OneVector);
		if (HeroGuard.Succeeded() && GuardMesh)
		{
			GuardMesh->SetStaticMesh(HeroGuard.Object);
			GuardMesh->SetRelativeLocation(FVector(0.0f, -19.2f, 0.0f));
			GuardMesh->SetRelativeRotation(HangDown);
			GuardMesh->SetRelativeScale3D(FVector::OneVector);
		}
		if (HeroHandle.Succeeded() && HandleMesh)
		{
			HandleMesh->SetStaticMesh(HeroHandle.Object);
			HandleMesh->SetRelativeLocation(FVector(0.0f, -0.9f, 0.0f));
			HandleMesh->SetRelativeRotation(HangDown);
			HandleMesh->SetRelativeScale3D(FVector::OneVector);
		}
		if (HeroTip.Succeeded() && TipMesh)
		{
			TipMesh->SetStaticMesh(HeroTip.Object);
			TipMesh->SetRelativeLocation(FVector(0.0f, 14.6f, 0.0f));
			TipMesh->SetRelativeRotation(HangDown);
			TipMesh->SetRelativeScale3D(FVector::OneVector);
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

void ASword::DropPhysics()
{
	// No longer wielded: stop hit queries, drop the ownership chain that
	// routes kill credit, and leave the wielder's hand.
	bHitEnabled = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	OwnerActor = nullptr;
	SetOwner(nullptr);

	if (BladeCollision)
	{
		// The root sphere becomes the physical body; the visual parts stay
		// non-physical children and ride along. Shrink it so the blade rests
		// close to the ground instead of balancing on a 30 cm sphere.
		BladeCollision->SetSphereRadius(6.0f);
		BladeCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BladeCollision->SetCollisionObjectType(ECC_PhysicsBody);
		BladeCollision->SetSimulatePhysics(true);
		// Inherit the last swing so a killed wielder's blade keeps its motion.
		BladeCollision->SetPhysicsLinearVelocity(SwingVelocity);
	}

	// Debris: despawn once the fight has moved on.
	SetLifeSpan(30.0f);
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
	if (TipMesh)
	{
		TipMesh->SetCastShadow(bEnabled);
		TipMesh->bCastHiddenShadow = bEnabled;
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
