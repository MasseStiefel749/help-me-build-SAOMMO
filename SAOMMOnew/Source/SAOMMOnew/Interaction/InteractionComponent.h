// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

/** Broadcast when an interaction trace hits an actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteraction, AActor*, HitActor, FVector, HitLocation);

/**
 *  Interaction component (Band 3 §6).
 *
 *  Performs a forward line trace from the owning actor and reports whatever it
 *  hits, so gameplay systems / Blueprints can implement pick up, activate,
 *  open, or talk-to behavior. Kept data-only here so it does not dictate how a
 *  specific interaction resolves.
 */
UCLASS(ClassGroup = (Game), meta = (BlueprintSpawnableComponent))
class UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInteractionComponent();

	/** Maximum interaction reach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float InteractionRange = 200.0f;

	/** Channel used for the interaction trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

	/** Broadcast when an interaction trace hits an actor. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteraction OnInteraction;

	/** Currently focused actor under the interaction trace (null if none). */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> FocusedActor = nullptr;

	/** Performs the interaction trace from the owner's view/forward direction. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	/** Returns the current focus target (updated every tick). */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetFocusedActor() const { return FocusedActor; }

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Runs the forward trace; returns hit actor or null. Updates FocusedActor. */
	AActor* UpdateFocus();
};
