// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "EnemySpawner.h"
#include "Enemy.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Enemy spawner roster contract (ADR-011b, Band 3 §9 Encounter):
 *  - StartSpawning() runs TrySpawn() synchronously, so the first enemy
 *    exists immediately on level start (no timer wait);
 *  - the roster never exceeds MaxAlive (TrySpawn prunes stale entries and
 *    then returns early once Spawned.Num() >= MaxAlive);
 *  - destroyed enemies leave the roster immediately (weak-ptr invalidation
 *    is what GetAliveCount() filters);
 *  - a subsequent StartSpawning() refills a free slot - the synchronous
 *    stand-in for the timer-driven top-up, which needs a ticking world
 *    and is therefore NOT covered here (documented test gap: repeating
 *    SpawnInterval top-ups are only exercised by play sessions);
 *  - OBSERVED SEMANTICS (documented, not endorsed - Band 6 §19): an enemy
 *    killed via TakeDamage() stays in the roster until its 2s lifespan
 *    destruction runs, because OnSpawnedDied() only prunes *invalid*
 *    entries and a ragdoll corpse is still a valid weak pointer. World
 *    time does not advance in a bare test world, so the corpse persists
 *    deterministically and both halves of the rule are assertable.
 *
 *  BeginPlay does not run in a bare world, so bAutoStart never fires here;
 *  the test sets bAutoStart=false explicitly to stay independent of that.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOEnemySpawnerRosterContractTest,
	"SAOMMOnew.Enemies.SpawnerRosterContract",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOEnemySpawnerRosterContractTest::RunTest(const FString& Parameters)
{
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	AEnemySpawner* Spawner = World->SpawnActor<AEnemySpawner>(
		FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Spawner spawned"), Spawner))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	Spawner->bAutoStart = false;
	Spawner->MaxAlive = 2;

	// Nothing spawns until StartSpawning() is called.
	TestEqual(TEXT("Roster empty before StartSpawning"), Spawner->GetAliveCount(), 0);

	// Synchronous first wave: TrySpawn() runs inside StartSpawning().
	Spawner->StartSpawning();
	TestEqual(TEXT("Immediate first spawn on StartSpawning"), Spawner->GetAliveCount(), 1);

	// Second call refills up to the cap; third call must not exceed it.
	Spawner->StartSpawning();
	TestEqual(TEXT("Refill reaches MaxAlive"), Spawner->GetAliveCount(), 2);
	Spawner->StartSpawning();
	TestEqual(TEXT("MaxAlive cap holds on further calls"), Spawner->GetAliveCount(), 2);

	// The world really contains that many AEnemy actors.
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemy::StaticClass(), Enemies);
	TestEqual(TEXT("Spawned actor count matches roster"), Enemies.Num(), 2);
	if (Enemies.Num() != 2)
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Destroying an enemy frees its slot immediately (weak ptr invalid).
	Enemies[0]->Destroy();
	TestEqual(TEXT("Destroyed enemy leaves the roster"), Spawner->GetAliveCount(), 1);

	// Refill through the public entry point - synchronous stand-in for the
	// timer top-up (the timer itself needs a ticking world, see header).
	Spawner->StartSpawning();
	TestEqual(TEXT("Free slot refilled via TrySpawn"), Spawner->GetAliveCount(), 2);

	// Kill one live enemy through the AActor base (AEnemy::TakeDamage is
	// protected). CurrentHealth is set explicitly: BeginPlay - which
	// copies MaxHealth - does not run in a bare world.
	TArray<AActor*> Live;
	UGameplayStatics::GetAllActorsOfClass(World, AEnemy::StaticClass(), Live);
	if (!TestEqual(TEXT("Two live enemies before the kill"), Live.Num(), 2))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	AEnemy* Victim = Cast<AEnemy>(Live[0]);
	AActor* VictimBase = Live[0];
	if (!TestNotNull(TEXT("Victim castable to AEnemy"), Victim))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	Victim->CurrentHealth = 1.0f;
	VictimBase->TakeDamage(10000.0f, FDamageEvent(), nullptr, nullptr);

	// Public observables that Die() ran: health hit zero and the corpse
	// lifespan (2s) was set. bDead itself is protected (Enemy.h).
	TestTrue(TEXT("Victim health drained by the killing blow"), Victim->CurrentHealth <= 0.0f);
	TestTrue(TEXT("Die() ran (2s corpse lifespan set)"),
		FMath::IsNearlyEqual(Victim->GetLifeSpan(), 2.0f, 0.01f));
	// OBSERVED: the corpse still occupies its slot - OnSpawnedDied only
	// prunes invalid entries, and the lifespan (2s) has not elapsed
	// because world time never advances here.
	TestEqual(TEXT("Corpse keeps its roster slot until destruction"),
		Spawner->GetAliveCount(), 2);

	// Destroying the corpse frees the slot - the other half of the rule.
	Victim->Destroy();
	TestEqual(TEXT("Destroyed corpse frees the slot"), Spawner->GetAliveCount(), 1);

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
