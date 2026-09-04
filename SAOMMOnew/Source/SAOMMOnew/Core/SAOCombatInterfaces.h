// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SAOCombatInterfaces.generated.h"

/**
 *  Target-architecture combat attacker interface (Band 2 §8).
 *  Counterpart to the Variant_Combat interfaces; lives in Core so Combat,
 *  Enemies, Character, and VR all share one contract.
 */
UINTERFACE(MinimalAPI, NotBlueprintable)
class USAOCombatAttacker : public UInterface
{
	GENERATED_BODY()
};

class ISAOCombatAttacker
{
	GENERATED_BODY()

public:

	/** Performs an attack's collision check. Usually called from a montage AnimNotify or a swing-detection system. */
	UFUNCTION(BlueprintCallable, Category = "Attacker")
	virtual void DoAttackTrace(FName DamageSourceBone) = 0;

	/** Performs a combo string continuation check. */
	UFUNCTION(BlueprintCallable, Category = "Attacker")
	virtual void CheckCombo() = 0;
};

/**
 *  Target-architecture damageable interface (Band 2 §8, §10).
 */
UINTERFACE(MinimalAPI, NotBlueprintable)
class USAOCombatDamageable : public UInterface
{
	GENERATED_BODY()
};

class ISAOCombatDamageable
{
	GENERATED_BODY()

public:

	/** Handles damage and knockback events. */
	UFUNCTION(BlueprintCallable, Category = "Damageable")
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) = 0;

	/** Handles death events. */
	UFUNCTION(BlueprintCallable, Category = "Damageable")
	virtual void HandleDeath() = 0;

	/** Handles healing events. */
	UFUNCTION(BlueprintCallable, Category = "Damageable")
	virtual void ApplyHealing(float Healing, AActor* Healer) = 0;

	/** Notifies the actor of impending danger so it can react. */
	UFUNCTION(BlueprintCallable, Category = "Damageable")
	virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) = 0;
};
