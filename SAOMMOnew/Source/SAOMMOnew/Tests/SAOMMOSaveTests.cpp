// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerSaveGame.h"
#include "ItemTypes.h"
#include "SAOMMOTestWorld.h"
#include "Checkpoint.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "ProgressionComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Controller.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Save/load roundtrip (#19 Testinfrastruktur, Band 2 §15): the USaveGame
 *  wrapper must survive a serialize/deserialize cycle without losing player
 *  position, health, level, experience or inventory. The roundtrip runs
 *  entirely in memory (SaveGameToMemory/LoadGameFromMemory) so the test
 *  writes no files — matching the "leave nothing on disk" test rule.
 *  Identity-valued fields (SlotName/UserIndex defaults) are not asserted
 *  because they would pass trivially.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOSavePlayerSaveRoundtripTest,
	"SAOMMOnew.Save.PlayerSaveRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOSavePlayerSaveRoundtripTest::RunTest(const FString& Parameters)
{
	UPlayerSaveGame* Original = NewObject<UPlayerSaveGame>();
	if (!TestNotNull(TEXT("Save object created"), Original))
	{
		return false;
	}

	Original->PlayerLocation = FVector(123.5, -45.25, 77.0);
	Original->PlayerRotation = FRotator(0.0, 90.0, 0.0);
	Original->PlayerHealth = 72.5f;
	Original->Level = 4;
	Original->Experience = 1234.5f;

	FInventoryItem Sword;
	Sword.ItemId = FName(TEXT("Eisenfaust"));
	Sword.Count = 1;
	Sword.Type = EItemType::Weapon;
	FInventoryItem Potion;
	Potion.ItemId = FName(TEXT("HealthPotion"));
	Potion.Count = 9;
	Potion.Type = EItemType::Consumable;
	Original->Inventory = { Sword, Potion };

	TArray<uint8> Bytes;
	const bool bWrote = UGameplayStatics::SaveGameToMemory(Original, Bytes);
	if (!TestTrue(TEXT("SaveGameToMemory produces bytes"), bWrote && Bytes.Num() > 0))
	{
		return false;
	}

	USaveGame* LoadedBase = UGameplayStatics::LoadGameFromMemory(Bytes);
	UPlayerSaveGame* Loaded = Cast<UPlayerSaveGame>(LoadedBase);
	if (!TestNotNull(TEXT("Loaded object is a UPlayerSaveGame"), Loaded))
	{
		return false;
	}

	TestTrue(TEXT("Location survives the roundtrip"),
		Loaded->PlayerLocation.Equals(Original->PlayerLocation, 0.001));
	TestTrue(TEXT("Rotation survives the roundtrip"),
		Loaded->PlayerRotation.Equals(Original->PlayerRotation, 0.001));
	TestTrue(TEXT("Health survives the roundtrip"),
		FMath::IsNearlyEqual(Loaded->PlayerHealth, 72.5f));
	TestEqual(TEXT("Level survives the roundtrip"), Loaded->Level, 4);
	TestTrue(TEXT("Experience survives the roundtrip"),
		FMath::IsNearlyEqual(Loaded->Experience, 1234.5f));

	TestEqual(TEXT("Inventory entry count survives"), Loaded->Inventory.Num(), Original->Inventory.Num());
	if (Loaded->Inventory.Num() == 2)
	{
		TestTrue(TEXT("Item id survives"), Loaded->Inventory[0].ItemId == Original->Inventory[0].ItemId);
		TestTrue(TEXT("Item type survives"), Loaded->Inventory[1].Type == EItemType::Consumable);
		TestEqual(TEXT("Stack count survives"), Loaded->Inventory[1].Count, 9);
	}

	return true;
}

/**
 *  Checkpoint -> save -> load -> respawn roundtrip (ADR-011a + ADR-011c,
 *  Band 3 §11/§14): a checkpoint overlap writes the respawn pose, SaveProgress
 *  persists pawn pose/health/inventory/progression to a dedicated test slot,
 *  and LoadProgress restores all of it and adopts the loaded pose as the new
 *  respawn transform ("loading doubles as a checkpoint"). Runs in a bare test
 *  world; the slot is deleted again so the test leaves nothing on disk and
 *  never touches the real "PlayerSave" slot.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOSaveCheckpointSaveLoadRespawnRoundtripTest,
	"SAOMMOnew.Save.CheckpointSaveLoadRespawnRoundtrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOSaveCheckpointSaveLoadRespawnRoundtripTest::RunTest(const FString& Parameters)
{
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	APlayerCharacter* Character = World->SpawnActor<APlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AMainPlayerController* Controller = World->SpawnActor<AMainPlayerController>();
	if (!TestNotNull(TEXT("Player character spawned"), Character)
		|| !TestNotNull(TEXT("MainPlayerController spawned"), Controller))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	Controller->Possess(Character);

	// 1) Checkpoint overlap writes the respawn pose (ADR-011a) — the flow the
	//    existing CheckpointRespawnFlow test starts; here it is the setup for
	//    the save/load half of the chain.
	const FRotator CheckpointRotation(0.0f, 45.0f, 0.0f);
	const FVector CheckpointLocation(1000.0f, 2000.0f, 50.0f);
	ACheckpoint* Checkpoint = World->SpawnActor<ACheckpoint>(CheckpointLocation, CheckpointRotation);
	if (TestNotNull(TEXT("Checkpoint spawned"), Checkpoint))
	{
		AActor* CheckpointAsActor = Checkpoint; // public dispatch entry (engine does the same)
		CheckpointAsActor->NotifyActorBeginOverlap(Character);
		const FTransform FromCheckpoint = Controller->GetRespawnTransform();
		TestTrue(TEXT("Checkpoint wrote its location into the respawn transform"),
			FromCheckpoint.GetLocation().Equals(CheckpointLocation, 0.01f));
		TestTrue(TEXT("Checkpoint wrote its facing into the respawn transform"),
			FQuat(FromCheckpoint.GetRotation()).Equals(CheckpointRotation.Quaternion(), 1.0e-4f));
	}

	// 2) Pose the pawn where the save should pick it up (distinct from the
	//    checkpoint pose so the two cannot be confused below).
	const FVector SavedLocation(300.0f, 400.0f, 60.0f);
	const float SavedYaw = 120.0f;
	Character->SetActorLocationAndRotation(SavedLocation, FRotator(0.0f, SavedYaw, 0.0f));
	Character->SetHealth(6.0f); // MaxHealth default is 10.
	Character->GetProgression()->SetProgress(3, 42.5f);
	FInventoryItem Sword;
	Sword.ItemId = FName(TEXT("Eisenfaust"));
	Sword.Count = 1;
	Sword.Type = EItemType::Weapon;
	Character->GetInventory()->SetItems({ Sword });

	TestTrue(TEXT("Pawn sits at the to-be-saved pose"),
		Character->GetActorLocation().Equals(SavedLocation, 0.01f));
	TestTrue(TEXT("Health is at the to-be-saved value"), FMath::IsNearlyEqual(Character->GetHealth(), 6.0f));

	// Dedicated test slot: never the real "PlayerSave", deleted at the end.
	const FString TestSlot = TEXT("SAOMMO_AutoTest_RespawnRoundtrip");
	Controller->SaveSlotName = TestSlot;
	const bool bSaved = Controller->SaveProgress();
	if (!TestTrue(TEXT("SaveProgress writes the test slot"), bSaved))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false; // nothing was written, so there is nothing to clean up
	}

	// 3) Mutate everything after the save — the load must undo all of it.
	Character->SetActorLocationAndRotation(FVector(9999.0f, 9999.0f, 1000.0f), FRotator::ZeroRotator);
	Character->SetHealth(1.0f);
	Character->GetProgression()->SetProgress(9, 1.0f);
	Character->GetInventory()->SetItems(TArray<FInventoryItem>());

	const bool bLoaded = Controller->LoadProgress();
	TestTrue(TEXT("LoadProgress loads the test slot"), bLoaded);
	if (bLoaded)
	{
		TestTrue(TEXT("Pawn teleports back to the saved pose"),
			Character->GetActorLocation().Equals(SavedLocation, 0.01f));
		TestTrue(TEXT("Pawn facing restores the saved yaw"),
			FMath::IsNearlyEqual(Character->GetActorRotation().Yaw, SavedYaw, 0.01f));
		TestTrue(TEXT("Health restores the saved value"),
			FMath::IsNearlyEqual(Character->GetHealth(), 6.0f));
		TestEqual(TEXT("Level restores the saved value"), Character->GetProgression()->Level, 3);
		TestTrue(TEXT("XP restores the saved value"),
			FMath::IsNearlyEqual(Character->GetProgression()->Experience, 42.5f));
		TestEqual(TEXT("Inventory restores the saved items"), Character->GetInventory()->Items.Num(), 1);

		// ADR-011c: loading doubles as a checkpoint — the respawn point
		// becomes the loaded pose (unit scale), not the old checkpoint pose.
		const FTransform AfterLoad = Controller->GetRespawnTransform();
		TestTrue(TEXT("Respawn transform adopts the loaded pose"),
			AfterLoad.GetLocation().Equals(SavedLocation, 0.01f));
		TestTrue(TEXT("Respawn scale stays unit"),
			AfterLoad.GetScale3D().Equals(FVector::OneVector));
	}

	// 4) Leave nothing on disk (test rule) — the real slot was never touched.
	TestTrue(TEXT("Test slot deletes cleanly"), UGameplayStatics::DeleteGameInSlot(TestSlot, 0));
	TestFalse(TEXT("Test slot is gone"), UGameplayStatics::DoesSaveGameExist(TestSlot, 0));

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
