// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAOMMOInteractionComponent.generated.h"

/** Broadcast when an interaction trace hits an actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSAOInteraction, AActor*, HitActor, FVector, HitLocation);

/**
 *  Interaction component (Band 3 §6).
 *
 *  Performs a forward line trace from the owning actor and reports whatever it
 *  hits, so gameplay systems / Blueprints can implement pick up, activate,
 *  open, or talk-to behavior. Kept data-only here so it does not dictate how a
 *  specific interaction resolves.
 */
UCLASS(ClassGroup = (SAOMMO), meta = (BlueprintSpawnableComponent))
class USAOMMOInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USAOMMOInteractionComponent();

	/** Maximum interaction reach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float InteractionRange = 200.0f;

	/** Channel used for the interaction trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionChannel = ECC_Visibility;

	/** Broadcast when an interaction trace hits an actor. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnSAOInteraction OnInteraction;

	/** Performs the interaction trace from the owner's view/forward direction. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

protected:

	virtual void BeginPlay() override;
};
