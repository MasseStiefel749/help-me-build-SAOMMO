// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProgressionComponent.h"

UProgressionComponent::UProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

int32 UProgressionComponent::AddExperience(float Amount)
{
	if (Amount <= 0.0f)
	{
		return Level;
	}

	Experience += Amount;
	CheckLevelUp();
	return Level;
}

void UProgressionComponent::SetProgress(int32 NewLevel, float NewExperience)
{
	const int32 OldLevel = Level;
	Level = FMath::Max(1, NewLevel);
	Experience = FMath::Max(0.0f, NewExperience);
	if (Level != OldLevel)
	{
		OnLevelUp.Broadcast(Level);
	}
}

void UProgressionComponent::CheckLevelUp()
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
