// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOInputFrameComponent.h"

USAOMMOInputFrameComponent::USAOMMOInputFrameComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void USAOMMOInputFrameComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnFrameUpdated.Broadcast(CurrentFrame);
}
