// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
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
 *
 *  Die Welt registriert zusaetzlich einen FWorldContext bei GEngine (Muster:
 *  Engine CQTest ActorTestSpawner, AutomationCommon::FTestWorldWrapper).
 *  Ohne den Kontext warnet UWorld::DestroyActor "World has no context!"
 *  bei jedem Actor, den die Tests zerstoeren (backlog #42).
 */
namespace SAOMMOTest
{
	/** Creates a bare game world and registers its FWorldContext with the engine. */
	inline UWorld* CreateTestWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);

		// Weltkontext registrieren - sonst hat DestroyActor keinen Kontext (#42).
		// GEngine laeuft im Testbetrieb immer; ohne Engine bleibt die Welt
		// kontextlos wie zuvor (kein Abbruch, Tests verhalten sich gleich).
		if (World && GEngine)
		{
			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			WorldContext.SetCurrentWorld(World);
		}

		return World;
	}

	/** Tears the test world down (context removal + CleanupWorld + unroot). */
	inline void DestroyTestWorld(UWorld* World)
	{
		if (World)
		{
			// Erst den Kontext abbauen, dann die Welt - DestroyWorldContext ist
			// ein no-op, wenn kein Kontext existiert (z.B. GEngine fehlte).
			if (GEngine)
			{
				GEngine->DestroyWorldContext(World);
			}

			World->DestroyWorld(false);
		}
	}
}

#endif // WITH_AUTOMATION_TESTS
