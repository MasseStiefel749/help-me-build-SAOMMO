// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InventoryComponent.h"
#include "ItemTypes.h"
#include "SAOMMOTestWorld.h"
#include "PlayerCharacter.h"
#include "ItemPickup.h"
#include "GameFramework/DefaultPawn.h"

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

/**
 *  Item pickup roundtrip (ADR-010d Fight->Loot, Band 3 §13): a successful
 *  TryPickup transfers the configured stack into the caller's
 *  UInventoryComponent and destroys the pickup; every rejection path
 *  (null caller, ItemId None, Count <= 0, pawn without an inventory)
 *  returns false and LEAVES the pickup in place.
 *
 *  Documented gaps (not covered here): the controller-owned-pawn
 *  resolution branch (ItemPickup.cpp:76-86) needs a pawn/controller
 *  split a bare test world cannot express meaningfully; the walk-over
 *  overlap path (OnPickupOverlap) needs a ticking world -> PIE
 *  (backlog #2).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOItemPickupRoundtripTest,
	"SAOMMOnew.Inventory.PickupRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOItemPickupRoundtripTest::RunTest(const FString& Parameters)
{
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	APlayerCharacter* Character = World->SpawnActor<APlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Player character spawned"), Character))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	UInventoryComponent* Inv = Character->GetInventory();
	if (!TestNotNull(TEXT("Character owns an inventory component"), Inv))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	TestEqual(TEXT("Fresh inventory is empty"), Inv->Items.Num(), 0);

	AItemPickup* Pickup = World->SpawnActor<AItemPickup>(FVector(3000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Pickup spawned"), Pickup))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Configure() applies id + count (guards None / <= 0 itself).
	Pickup->Configure(FName(TEXT("HealthHerb")), 3);
	TestEqual(TEXT("Configure set the item id"), Pickup->Item.ItemId, FName(TEXT("HealthHerb")));
	TestEqual(TEXT("Configure set the count"), Pickup->Item.Count, 3);

	// --- Success path: stack lands, pickup destroys itself. ---
	TestTrue(TEXT("TryPickup succeeds for an inventoried pawn"), Pickup->TryPickup(Character));
	TestEqual(TEXT("One stack added"), Inv->Items.Num(), 1);
	TestEqual(TEXT("Transferred stack id"), Inv->Items[0].ItemId, FName(TEXT("HealthHerb")));
	TestEqual(TEXT("Transferred stack count"), Inv->Items[0].Count, 3);
	TestEqual(TEXT("GetItemCount mirrors the pickup"),
		Inv->GetItemCount(FName(TEXT("HealthHerb"))), 3);
	TestFalse(TEXT("Pickup destroyed itself after success"), IsValid(Pickup));

	// --- Rejection paths: false AND the pickup stays in the world. ---
	AItemPickup* P2 = World->SpawnActor<AItemPickup>(FVector(3400.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Second pickup spawned"), P2))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	TestFalse(TEXT("TryPickup(nullptr) rejects"), P2->TryPickup(nullptr));
	TestTrue(TEXT("...and the pickup survives"), IsValid(P2));

	P2->Item.Count = 0;
	TestFalse(TEXT("TryPickup with Count 0 rejects"), P2->TryPickup(Character));
	TestTrue(TEXT("...and the pickup survives"), IsValid(P2));
	P2->Item.Count = 2;

	P2->Item.ItemId = NAME_None;
	TestFalse(TEXT("TryPickup with ItemId None rejects"), P2->TryPickup(Character));
	TestTrue(TEXT("...and the pickup survives"), IsValid(P2));
	P2->Item.ItemId = FName(TEXT("HealthHerb"));

	ADefaultPawn* Bare = World->SpawnActor<ADefaultPawn>(FVector(3800.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Bare pawn without inventory spawned"), Bare))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	TestFalse(TEXT("TryPickup on a pawn without inventory rejects"), P2->TryPickup(Bare));
	TestTrue(TEXT("...and the pickup survives"), IsValid(P2));

	// All rejections left the inventored pawn untouched.
	TestEqual(TEXT("Inventoried pawn still holds only the granted stack"),
		Inv->GetItemCount(FName(TEXT("HealthHerb"))), 3);

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
