// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOInteractionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SAOItemPickup.h"

USAOMMOInteractionComponent::USAOMMOInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void USAOMMOInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USAOMMOInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFocus();
}

AActor* USAOMMOInteractionComponent::UpdateFocus()
{
	FocusedActor = nullptr;

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return nullptr;
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start + Owner->GetActorForwardVector() * InteractionRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (World->LineTraceSingleByChannel(Hit, Start, End, InteractionChannel, Params))
	{
		FocusedActor = Hit.GetActor();
	}
	return FocusedActor;
}

void USAOMMOInteractionComponent::Interact()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start + Owner->GetActorForwardVector() * InteractionRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (World->LineTraceSingleByChannel(Hit, Start, End, InteractionChannel, Params))
	{
		FocusedActor = Hit.GetActor();
		// Resolve pickups directly so Interact = pick up without Blueprint glue.
		if (ASAOItemPickup* Pickup = Cast<ASAOItemPickup>(Hit.GetActor()))
		{
			Pickup->TryPickup(Owner);
		}
		OnInteraction.Broadcast(Hit.GetActor(), Hit.ImpactPoint);
	}
}
