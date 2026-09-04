// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

int32 UInventoryComponent::FindItemIndex(FName ItemId) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemId == ItemId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::AddItem(const FInventoryItem& Item)
{
	if (Item.ItemId.IsNone() || Item.Count <= 0)
	{
		return 0;
	}

	const int32 Index = FindItemIndex(Item.ItemId);
	if (Index != INDEX_NONE)
	{
		Items[Index].Count += Item.Count;
	}
	else
	{
		Items.Add(Item);
	}

	OnInventoryChanged.Broadcast();
	return GetItemCount(Item.ItemId);
}

int32 UInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	const int32 Index = FindItemIndex(ItemId);
	if (Index == INDEX_NONE)
	{
		return 0;
	}

	const int32 Removed = FMath::Min(Count, Items[Index].Count);
	Items[Index].Count -= Removed;

	if (Items[Index].Count <= 0)
	{
		Items.RemoveAt(Index);
	}

	OnInventoryChanged.Broadcast();
	return Removed;
}

int32 UInventoryComponent::GetItemCount(FName ItemId) const
{
	const int32 Index = FindItemIndex(ItemId);
	return Index != INDEX_NONE ? Items[Index].Count : 0;
}

void UInventoryComponent::SetItems(const TArray<FInventoryItem>& NewItems)
{
	Items.Reset();
	for (const FInventoryItem& Item : NewItems)
	{
		if (!Item.ItemId.IsNone() && Item.Count > 0)
		{
			Items.Add(Item);
		}
	}
	OnInventoryChanged.Broadcast();
}
