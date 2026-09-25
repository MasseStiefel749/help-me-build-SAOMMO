// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InventoryComponent.h"
#include "ItemTypes.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Inventory boundaries (#19 Testinfrastruktur, Band 3 §13): the band
 *  specifies a flat list of item stacks and no explicit max-stack cap, so the
 *  current semantics are pinned here: invalid entries rejected, same-id adds
 *  stack into a single entry, partial removal trims, removal at/below the held
 *  count drops the entry, and SetItems (the load path) filters junk. If a
 *  stack cap is specified later, this is the test to extend.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOInventoryStackBoundariesTest,
	"SAOMMOnew.Inventory.StackBoundaries",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOInventoryStackBoundariesTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = NewObject<UInventoryComponent>();
	if (!TestNotNull(TEXT("Inventory component created"), Inv))
	{
		return false;
	}

	// --- Invalid input is rejected without touching the list. ---
	FInventoryItem NoId;
	NoId.ItemId = NAME_None;
	NoId.Count = 5;
	TestEqual(TEXT("Add without ItemId returns 0"), Inv->AddItem(NoId), 0);
	TestEqual(TEXT("...and the list stays empty"), Inv->Items.Num(), 0);

	FInventoryItem NoCount;
	NoCount.ItemId = FName(TEXT("Rag"));
	NoCount.Count = 0;
	TestEqual(TEXT("Add with Count 0 returns 0"), Inv->AddItem(NoCount), 0);
	NoCount.Count = -3;
	TestEqual(TEXT("Add with negative Count returns 0"), Inv->AddItem(NoCount), 0);
	TestEqual(TEXT("List still empty after invalid adds"), Inv->Items.Num(), 0);

	// --- Valid adds stack into ONE entry per id (no cap specified yet). ---
	FInventoryItem Potion;
	Potion.ItemId = FName(TEXT("HealthPotion"));
	Potion.Count = 3;
	Potion.Type = EItemType::Consumable;
	TestEqual(TEXT("First add returns 3"), Inv->AddItem(Potion), 3);
	Potion.Count = 4;
	TestEqual(TEXT("Second add stacks to 7"), Inv->AddItem(Potion), 7);
	TestEqual(TEXT("Stacking keeps a single entry"), Inv->Items.Num(), 1);
	TestEqual(TEXT("GetItemCount mirrors the stack"), Inv->GetItemCount(FName(TEXT("HealthPotion"))), 7);

	// --- Removal: trims, caps at the held count, drops empty entries. ---
	TestEqual(TEXT("Remove 4 of 7 returns 4"), Inv->RemoveItem(FName(TEXT("HealthPotion")), 4), 4);
	TestEqual(TEXT("3 remain after partial removal"), Inv->GetItemCount(FName(TEXT("HealthPotion"))), 3);
	TestEqual(TEXT("Remove beyond held count returns what was held"),
		Inv->RemoveItem(FName(TEXT("HealthPotion")), 10), 3);
	TestEqual(TEXT("Entry dropped once the stack empties"), Inv->Items.Num(), 0);
	TestEqual(TEXT("Removing an absent id returns 0"), Inv->RemoveItem(FName(TEXT("NoSuchItem")), 1), 0);

	// --- SetItems (save/load path) filters junk entries. ---
	TArray<FInventoryItem> Loaded;
	FInventoryItem Ok;
	Ok.ItemId = FName(TEXT("IronSword"));
	Ok.Count = 1;
	Ok.Type = EItemType::Weapon;
	Loaded.Add(Ok);
	Loaded.Add(NoId); // None id
	FInventoryItem Zero;
	Zero.ItemId = FName(TEXT("Trash"));
	Zero.Count = 0;
	Loaded.Add(Zero);
	Inv->SetItems(Loaded);
	TestEqual(TEXT("SetItems keeps only valid entries"), Inv->Items.Num(), 1);
	TestEqual(TEXT("Surviving entry is the valid one"), Inv->GetItemCount(FName(TEXT("IronSword"))), 1);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
