// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAOCombatInterfaces.h"
#include "SAOEnemy.generated.h"

/** Initial enemy behavior states (Band 2 §11, Band 3 §10). */
UENUM(BlueprintType)
enum class ESAOEnemyState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Detect		UMETA(DisplayName = "Detect Player"),
	Approach	UMETA(DisplayName = "Move Toward Player"),
	Attack		UMETA(DisplayName = "Attack"),
	Recover		UMETA(DisplayName = "Recover")
};

/** Broadcast when the enemy dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSAOEnemyDied);

/**
 *  Basic AI enemy (Band 2 §10, §11).
 *
 *  This first prototype uses a lightweight tick-driven state machine instead of
 *  BehaviorTree / NavMesh so it can be reasoned about and tested without
 *  navigation assets. The architecture separates body, health, and behavior so
 *  a full AI Controller / BehaviorTree can replace the FSM later.
 */
UCLASS(Blueprintable)
class ASAOEnemy : public ACharacter, public ISAOCombatDamageable
{
	GENERATED_BODY()

public:

	ASAOEnemy();

	/** Maximum health on spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 1, ClampMax = 1000))
	float MaxHealth = 3.0f;

	/** Current health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float CurrentHealth = 0.0f;

	/** Distance at which the enemy becomes aware of the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm"))
	float DetectRange = 1000.0f;

	/** Distance at which the enemy attempts an attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float AttackRange = 120.0f;

	/** Movement speed while approaching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float ApproachSpeed = 200.0f;

	/** Damage dealt per attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 1000))
	float AttackDamage = 1.0f;

	/** XP granted to the killer's progression component on death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards", meta = (ClampMin = 0))
	float XPReward = 10.0f;

	/** Item granted to the killer's inventory on death (Count<=0 or empty id = none). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName LootItemId;

	/** How many of LootItemId to grant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards", meta = (ClampMin = 0))
	int32 LootCount = 1;

	/** Time the enemy spends recovering after an attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RecoverTime = 1.0f;

	/** Broadcast when the enemy dies. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSAOEnemyDied OnDied;

	/** Returns the current AI state. */
	UFUNCTION(BlueprintCallable, Category = "AI")
	ESAOEnemyState GetState() const { return State; }

protected:

	/** Current behavior state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	ESAOEnemyState State = ESAOEnemyState::Idle;

	/** Countdown used by the Recover state. */
	float RecoverRemaining = 0.0f;

	/** Attack cooldown guard. */
	bool bAttackReady = true;

	/** Last known player pawn, cached during detection. */
	UPROPERTY()
	TWeakObjectPtr<APawn> TargetPawn;

	/** True once Die() ran; guards the TakeDamage/ApplyDamage double-death. */
	bool bDead = false;

	/** Controller credited with the kill (set from TakeDamage/ApplyDamage instigator). */
	UPROPERTY()
	TWeakObjectPtr<AController> Killer;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	/** Resolves the current state and transitions to the next. */
	void UpdateState(float DeltaTime);

	/** Moves the enemy toward the target without requiring a Navigation mesh. */
	void MoveTowardTarget(float DeltaTime);

	/** Performs a single melee attack against the target. */
	void PerformAttack();

	/** Handles death and removal from the level. */
	void Die();

	// ~begin ISAOCombatDamageable interface

	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
	virtual void HandleDeath() override;
	virtual void ApplyHealing(float Healing, AActor* Healer) override;
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

	// ~end ISAOCombatDamageable interface
};
