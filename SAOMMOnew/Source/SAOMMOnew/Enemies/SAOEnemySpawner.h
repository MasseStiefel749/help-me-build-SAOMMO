// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAOEnemySpawner.generated.h"

class ASAOEnemy;

/**
 *  Data-driven enemy spawner (Band 3 §9 Encounter, Starting Reach counts).
 *
 *  Place in a level (e.g. Grauwaldrand: 2-4 scavengers). Maintains up to
 *  MaxAlive enemies inside SpawnRadius, respawning on SpawnInterval while
 *  the spawner is active. Dead enemies release their slot via OnDied.
 */
UCLASS(Blueprintable)
class ASAOEnemySpawner : public AActor
{
	GENERATED_BODY()

public:

	ASAOEnemySpawner();

	/** Enemy class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<ASAOEnemy> EnemyClass;

	/** Maximum simultaneous live enemies from this spawner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = 0))
	int32 MaxAlive = 3;

	/** Seconds between population top-ups. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = 0.1, Units = "s"))
	float SpawnInterval = 5.0f;

	/** Radius around the spawner in which enemies appear. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = 0, Units = "cm"))
	float SpawnRadius = 500.0f;

	/** Start spawning on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bAutoStart = true;

	/** Starts the spawn timer (safe to call twice). */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void StartSpawning();

	/** Stops the spawn timer; live enemies are left alone. */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void StopSpawning();

	/** Current live enemy count. */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 GetAliveCount() const;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Spawns one enemy if a slot is free. */
	void TrySpawn();

	/** Drops dead enemies from the roster. */
	UFUNCTION()
	void OnSpawnedDied();

	/** Live roster (weak: dying enemies auto-null). */
	UPROPERTY()
	TArray<TWeakObjectPtr<ASAOEnemy>> Spawned;

	FTimerHandle SpawnTimerHandle;
};
