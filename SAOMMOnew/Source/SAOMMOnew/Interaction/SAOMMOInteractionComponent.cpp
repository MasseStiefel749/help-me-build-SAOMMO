// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOInteractionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USAOMMOInteractionComponent::USAOMMOInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void USAOMMOInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USAOMMOInteractionComponent::Interact()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start + Owner->GetActorForwardVector() * InteractionRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, InteractionChannel, Params))
	{
		OnInteraction.Broadcast(Hit.GetActor(), Hit.ImpactPoint);
	}
}
