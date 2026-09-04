// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputFrameComponent.h"

UInputFrameComponent::UInputFrameComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UInputFrameComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnFrameUpdated.Broadcast(CurrentFrame);
}
