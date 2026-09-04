// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "ItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  World item pickup (Band 3 §13 Fight->Loot).
 *
 *  Placed in a level (or spawned by enemy death). When a pawn's interaction
 *  resolves it via TryPickup, the item stack transfers into the caller's
 *  UInventoryComponent and the pickup destroys itself.
 */
UCLASS(Blueprintable)
class AItemPickup : public AActor
{
	GENERATED_BODY()

public:

	AItemPickup();

	/** Item granted on pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FInventoryItem Item;

	/** Pickup radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = 0, Units = "cm"))
	float PickupRadius = 60.0f;

	/** Transfers Item into Caller's inventory. Returns false if Caller has no inventory. */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool TryPickup(AActor* Caller);

	/** Sets the granted item id/count (script and Editor friendly). */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void Configure(FName ItemId, int32 Count);

protected:

	/** Walk-over pickup: collects when a pawn carrying an inventory enters. */
	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	/** Overlap / interaction volume. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* PickupVolume;

	/** Optional visual mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
};
