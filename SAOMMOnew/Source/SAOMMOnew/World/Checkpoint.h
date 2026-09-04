// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  Respawn checkpoint (Band 3 §11).
 *
 *  Place in a level at safe spots (e.g. Brunnfeld spawn, Klingenhof entry).
 *  When an APlayerCharacter enters the volume, the owning player controller's
 *  respawn transform updates — death then returns here instead of the
 *  level start. No Blueprint glue required.
 */
UCLASS(Blueprintable)
class ACheckpoint : public AActor
{
	GENERATED_BODY()

public:

	ACheckpoint();

	/** Checkpoint radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (ClampMin = 0, Units = "cm"))
	float CheckpointRadius = 200.0f;

protected:

	/** Overlap volume. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CheckpointVolume;

	/** Tall marker pillar so checkpoints read as landmarks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BeaconMesh;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
