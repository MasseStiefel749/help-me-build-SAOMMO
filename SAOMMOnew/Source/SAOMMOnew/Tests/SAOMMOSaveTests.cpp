// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerSaveGame.h"
#include "ItemTypes.h"

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

#endif // WITH_AUTOMATION_TESTS
