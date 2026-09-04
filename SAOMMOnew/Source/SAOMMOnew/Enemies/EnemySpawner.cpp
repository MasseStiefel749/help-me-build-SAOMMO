// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawner.h"
#include "Enemy.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemyClass = AEnemy::StaticClass();
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		StartSpawning();
	}
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::StartSpawning()
{
	if (UWorld* World = GetWorld())
	{
		// Immediate first wave so encounters exist on level start.
		TrySpawn();
		World->GetTimerManager().SetTimer(SpawnTimerHandle, this,
			&AEnemySpawner::TrySpawn, SpawnInterval, true);
	}
}

void AEnemySpawner::StopSpawning()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

int32 AEnemySpawner::GetAliveCount() const
{
	int32 Alive = 0;
	for (const TWeakObjectPtr<AEnemy>& Weak : Spawned)
	{
		if (Weak.IsValid())
		{
			++Alive;
		}
	}
	return Alive;
}

void AEnemySpawner::TrySpawn()
{
	UWorld* World = GetWorld();
	if (!World || !*EnemyClass)
	{
		return;
	}

	// Prune stale entries before counting.
	Spawned.RemoveAll([](const TWeakObjectPtr<AEnemy>& Weak) { return !Weak.IsValid(); });

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

	if (AEnemy* Enemy = World->SpawnActor<AEnemy>(EnemyClass, Location, FRotator::ZeroRotator, Params))
	{
		Enemy->OnDied.AddDynamic(this, &AEnemySpawner::OnSpawnedDied);
		Spawned.Add(Enemy);
	}
}

void AEnemySpawner::OnSpawnedDied()
{
	// Slot frees on next TrySpawn via stale-entry pruning; also prune now
	// so GetAliveCount is exact for HUD/objectives.
	Spawned.RemoveAll([](const TWeakObjectPtr<AEnemy>& Weak) { return !Weak.IsValid(); });
}
