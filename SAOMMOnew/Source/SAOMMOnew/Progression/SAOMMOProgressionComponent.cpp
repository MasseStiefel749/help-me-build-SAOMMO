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

void USAOMMOProgressionComponent::SetProgress(int32 NewLevel, float NewExperience)
{
	const int32 OldLevel = Level;
	Level = FMath::Max(1, NewLevel);
	Experience = FMath::Max(0.0f, NewExperience);
	if (Level != OldLevel)
	{
		OnLevelUp.Broadcast(Level);
	}
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
