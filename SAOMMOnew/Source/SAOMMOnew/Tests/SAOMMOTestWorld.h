// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Shared helpers for the SAOMMOnew automation tests (#19 Testinfrastruktur).
 *
 *  Tests that need real actors (checkpoint overlap, damage application) run in
 *  their own bare UWorld instead of the editor's world, so they never touch the
 *  open level and can spawn/destroy actors freely. The world is created rooted
 *  (CreateWorld default) and torn down via DestroyWorld(), so nothing leaks
 *  into GC roots. Default initialization values are used: physics scene on,
 *  which the enemy death path (ragdoll) relies on.
 */
namespace SAOMMOTest
{
	/** Creates a bare game world that the engine does not track. */
	inline UWorld* CreateTestWorld()
	{
		return UWorld::CreateWorld(EWorldType::Game, false);
	}

	/** Tears the test world down (CleanupWorld + unroot). */
	inline void DestroyTestWorld(UWorld* World)
	{
		if (World)
		{
			World->DestroyWorld(false);
		}
	}
}

#endif // WITH_AUTOMATION_TESTS
