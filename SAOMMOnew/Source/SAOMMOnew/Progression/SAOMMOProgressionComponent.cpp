// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOProgressionComponent.h"

USAOMMOProgressionComponent::USAOMMOProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

int32 USAOMMOProgressionComponent::AddExperience(float Amount)
{
	if (Amount <= 0.0f)
	{
		return Level;
	}

	Experience += Amount;
	CheckLevelUp();
	return Level;
}

void USAOMMOProgressionComponent::CheckLevelUp()
{
	bool bLeveled = false;
	while (Experience >= ExperiencePerLevel)
	{
		Experience -= ExperiencePerLevel;
		++Level;
		bLeveled = true;
	}

	if (bLeveled)
	{
		OnLevelUp.Broadcast(Level);
	}
}
