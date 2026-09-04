// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ASword;
class UInputFrameComponent;

/** Broadcast when a swing is detected and the sword becomes armed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatSwing);

/** Broadcast when an armed sweep damages a target (for damage numbers / HUD). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatHit, AActor*, HitActor, float, AppliedDamage, FVector, HitLocation);

/**
 *  Combat component (Band 2 §8, Band 3 §7).
 *
 *  Kept separate from the character. Translates the device-independent input
 *  frame and the sword's swing velocity into an "armed" window during which the
 *  sword's hit detection is enabled. This distinguishes an intentional attack
 *  from incidental movement (Band 3 §7) without putting combat logic in the
 *  character class.
 */
UCLASS(ClassGroup = (Game), meta = (BlueprintSpawnableComponent))
class UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCombatComponent();

	/** Sword this component controls. Assign via SetSword or the editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<ASword> Sword;

	/** Swing speed (cm/s) at or above which a swing is considered an attack even without an attack input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 5000))
	float SwingArmThreshold = 250.0f;

	/** How long hit detection stays armed after a swing is detected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float ArmedDuration = 0.25f;

	/** Radius (cm) of the active melee query performed while armed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float HitReach = 130.0f;

	/** True while the sword is armed and can deal damage. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bArmed = false;

	/** Broadcast when a swing is detected. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatSwing OnSwingStarted;

	/** Broadcast per damaged target while armed. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatHit OnHit;

	/** Sets the controlled sword. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetSword(ASword* InSword) { Sword = InSword; }

protected:

	UPROPERTY()
	TObjectPtr<UInputFrameComponent> InputFrame;

	/** Remaining armed time. */
	float ArmTimer = 0.0f;

	/** Previous-frame attack intent, for edge detection. */
	bool bPrevAttack = false;

	/** Per-target timestamps used to throttle repeated hits while armed. */
	TMap<TObjectPtr<AActor>, float> LastHitTime;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
