// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "ItemPickup.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFocus();
}

/** Shared trace endpoints: pawn eye + view direction when available (aims
 *  with the camera, including first-person pitch), else actor forward. */
static void GetTraceEndpoints(AActor* Owner, float Range, FVector& OutStart, FVector& OutEnd)
{
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
		OutStart = ViewLocation;
		OutEnd = ViewLocation + ViewRotation.Vector() * Range;
		return;
	}

	OutStart = Owner->GetActorLocation();
	OutEnd = OutStart + Owner->GetActorForwardVector() * Range;
}

AActor* UInteractionComponent::UpdateFocus()
{
	FocusedActor = nullptr;

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return nullptr;
	}

	FVector Start, End;
	GetTraceEndpoints(Owner, InteractionRange, Start, End);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (World->LineTraceSingleByChannel(Hit, Start, End, InteractionChannel, Params))
	{
		FocusedActor = Hit.GetActor();
	}
	return FocusedActor;
}

void UInteractionComponent::Interact()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	FVector Start, End;
	GetTraceEndpoints(Owner, InteractionRange, Start, End);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (World->LineTraceSingleByChannel(Hit, Start, End, InteractionChannel, Params))
	{
		FocusedActor = Hit.GetActor();
		// Resolve pickups directly so Interact = pick up without Blueprint glue.
		if (AItemPickup* Pickup = Cast<AItemPickup>(Hit.GetActor()))
		{
			Pickup->TryPickup(Owner);
		}
		OnInteraction.Broadcast(Hit.GetActor(), Hit.ImpactPoint);
	}
}
