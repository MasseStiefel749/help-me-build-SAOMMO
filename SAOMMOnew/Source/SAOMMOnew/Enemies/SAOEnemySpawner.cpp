// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOEnemySpawner.h"
#include "SAOEnemy.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASAOEnemySpawner::ASAOEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemyClass = ASAOEnemy::StaticClass();
}

void ASAOEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartSpawning();
	}
}

void ASAOEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void ASAOEnemySpawner::StartSpawning()
{
	if (UWorld* World = GetWorld())
	{
		// Immediate first wave so encounters exist on level start.
		TrySpawn();
		World->GetTimerManager().SetTimer(SpawnTimerHandle, this,
			&ASAOEnemySpawner::TrySpawn, SpawnInterval, true);
	}
}

void ASAOEnemySpawner::StopSpawning()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

int32 ASAOEnemySpawner::GetAliveCount() const
{
	int32 Alive = 0;
	for (const TWeakObjectPtr<ASAOEnemy>& Weak : Spawned)
	{
		if (Weak.IsValid())
		{
			++Alive;
		}
	}
	return Alive;
}

void ASAOEnemySpawner::TrySpawn()
{
	UWorld* World = GetWorld();
	if (!World || !*EnemyClass)
	{
		return;
	}

	// Prune stale entries before counting.
	Spawned.RemoveAll([](const TWeakObjectPtr<ASAOEnemy>& Weak) { return !Weak.IsValid(); });

	if (Spawned.Num() >= MaxAlive)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
	const float Distance = FMath::RandRange(0.0f, SpawnRadius);
	const FVector Location = Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	if (ASAOEnemy* Enemy = World->SpawnActor<ASAOEnemy>(EnemyClass, Location, FRotator::ZeroRotator, Params))
	{
		Enemy->OnDied.AddDynamic(this, &ASAOEnemySpawner::OnSpawnedDied);
		Spawned.Add(Enemy);
	}
}

void ASAOEnemySpawner::OnSpawnedDied()
{
	// Slot frees on next TrySpawn via stale-entry pruning; also prune now
	// so GetAliveCount is exact for HUD/objectives.
	Spawned.RemoveAll([](const TWeakObjectPtr<ASAOEnemy>& Weak) { return !Weak.IsValid(); });
}
