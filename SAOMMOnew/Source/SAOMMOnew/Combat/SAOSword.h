// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAOSword.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/** Broadcast when the sword strikes an actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwordHit, AActor*, HitActor, float, Damage);

/**
 *  Weapon actor (Band 2 §8).
 *
 *  Responsibilities kept on the weapon rather than the character:
 *  - visual mesh
 *  - collision / hit detection
 *  - damage information
 *  - owner information
 *  - movement tracking (swing velocity, Band 3 §8)
 *
 *  First prototype uses an overlap volume for hit detection (Band 2 §9).
 */
UCLASS(Blueprintable)
class ASAOSword : public AActor
{
	GENERATED_BODY()

	/** Root and collision volume for hit detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* BladeCollision;

	/** Optional visual mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

public:

	ASAOSword();

	/** Damage dealt on a successful hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 1000))
	float Damage = 1.0f;

	/** When false, overlaps do not deal damage. The combat component arms this only during a real swing (Band 3 §7). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHitEnabled = true;

	/** Arms or disarms hit detection without destroying the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetHitEnabled(bool bEnabled) { bHitEnabled = bEnabled; }

	/** Minimum time between two hits against the same actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float PerTargetCooldown = 0.4f;

	/** Broadcast when the sword strikes an actor. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnSwordHit OnSwordHit;

	/** Returns the current swing velocity in cm/s (Band 3 §8). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FVector GetSwingVelocity() const { return SwingVelocity; }

	/** Sets the actor that owns this weapon (typically the wielder). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetOwnerActor(AActor* NewOwner)
	{
		OwnerActor = NewOwner;
		// Mirror into AActor ownership so GetInstigatorController() on the
		// sword resolves to the wielder (kill credit, damage causer chain).
		SetOwner(NewOwner);
	}

	/** Returns the wielding actor; hits against it are ignored. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	AActor* GetOwnerActor() const { return OwnerActor.Get(); }

protected:

	/** Actor that wields this weapon; hits against it are ignored. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TWeakObjectPtr<AActor> OwnerActor;

	/** Last frame world position, used to estimate swing velocity. */
	FVector PreviousLocation = FVector::ZeroVector;

	/** Estimated swing velocity in cm/s. */
	FVector SwingVelocity = FVector::ZeroVector;

	/** Per-target timestamps to throttle repeated hits (used by the combat component's active query). */
	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> LastHitTime;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
