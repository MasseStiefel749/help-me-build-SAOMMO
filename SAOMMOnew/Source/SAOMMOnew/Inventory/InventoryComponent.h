// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTypes.h"
#include "InventoryComponent.generated.h"

/** Broadcast when the inventory changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 *  Inventory component (Band 3 §13).
 *
 *  First implementation is intentionally simple: a flat list of item stacks.
 *  Weapon/armor/consumable/material/quest behaviour is layered on later by other
 *  systems; this component only owns and mutates the collection.
 */
UCLASS(ClassGroup = (Game), meta = (BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInventoryComponent();

	/** Current items. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> Items;

	/** Broadcast when Items change. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Adds (or stacks) an item. Returns the new total count for that item id. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(const FInventoryItem& Item);

	/** Removes up to Count of an item id. Returns how many were actually removed. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemId, int32 Count = 1);

	/** Returns the count held for an item id (0 if absent). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	/** Replaces the whole collection (save/load). Broadcasts once. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItems(const TArray<FInventoryItem>& NewItems);

protected:

	int32 FindItemIndex(FName ItemId) const;
};
