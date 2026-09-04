// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAOMMOProgressionComponent.generated.h"

/** Broadcast when the character levels up. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSAOLevelUp, int32, NewLevel);

/**
 *  Progression component (Band 3 §12).
 *
 *  Long-term progression (XP, levels, skills, equipment) must not be implemented
 *  before the core combat loop works. This minimal component exists so the loop
 *  has a place to record XP; it deliberately does no balancing.
 */
UCLASS(ClassGroup = (SAOMMO), meta = (BlueprintSpawnableComponent))
class USAOMMOProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USAOMMOProgressionComponent();

	/** Current level (1-based). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 Level = 1;

	/** Current experience toward the next level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	float Experience = 0.0f;

	/** Experience required to advance one level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression", meta = (ClampMin = 1))
	float ExperiencePerLevel = 100.0f;

	/** Broadcast when Level increases. */
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnSAOLevelUp OnLevelUp;

	/** Adds experience and resolves any level-ups. Returns the new level. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	int32 AddExperience(float Amount);

	/** Overwrites level/XP (save/load). Broadcasts only on level change. */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetProgress(int32 NewLevel, float NewExperience);

protected:

	void CheckLevelUp();
};
