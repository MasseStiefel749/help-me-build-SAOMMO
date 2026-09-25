// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "Enemy.h"
#include "ProgressionComponent.h"
#include "InventoryComponent.h"
#include "ItemTypes.h"
#include "PlayerHudWidget.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Kill reward flow (ADR-010b, Band 3 §12-§13): the killing blow routes the
 *  enemy's XPReward into the killer pawn's progression component and the
 *  configured loot into its inventory, and the bDead guard prevents a second
 *  blow from paying twice. Runs in a bare test world with kill credit passed
 *  explicitly as EventInstigator (the same channel the melee path resolves
 *  through the sword's instigator chain); enemy health is set explicitly
 *  because BeginPlay (which copies MaxHealth) does not run there.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOKillRewardXpAndLootTest,
	"SAOMMOnew.Combat.KillRewardXpAndLoot",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOKillRewardXpAndLootTest::RunTest(const FString& Parameters)
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

	UProgressionComponent* Prog = Character->GetProgression();
	UInventoryComponent* Inv = Character->GetInventory();
	if (!TestNotNull(TEXT("Character owns a progression component"), Prog)
		|| !TestNotNull(TEXT("Character owns an inventory component"), Inv))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Fresh character: no XP, no items (the reward assertions below are exact).
	TestEqual(TEXT("Fresh character is level 1"), Prog->Level, 1);
	TestTrue(TEXT("Fresh character has no XP"), FMath::IsNearlyZero(Prog->Experience));
	TestEqual(TEXT("Fresh inventory is empty"), Inv->Items.Num(), 0);

	AEnemy* Enemy = World->SpawnActor<AEnemy>(FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	Enemy->CurrentHealth = 1.0f; // BeginPlay does not run in a bare world.
	Enemy->LootItemId = FName(TEXT("HealthHerb"));
	Enemy->LootCount = 2;
	const float Reward = Enemy->XPReward;
	if (!TestTrue(TEXT("Enemy XP reward is positive"), Reward > 0.0f))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Lethal blow with kill credit (ADR-010b: EventInstigator is the channel
	// the melee path resolves through the sword's instigator controller).
	// AEnemy re-declares TakeDamage as protected — call through the public
	// AActor entry point, exactly like UGameplayStatics::ApplyDamage does.
	AActor* EnemyAsActor = Enemy;
	const float Dealt = EnemyAsActor->TakeDamage(50.0f, FDamageEvent(), Controller, nullptr);
	TestTrue(TEXT("Killing blow deals damage"), Dealt > 0.0f);
	TestTrue(TEXT("Enemy health clamps at 0"),
		FMath::IsNearlyEqual(Enemy->CurrentHealth, 0.0f));

	// XP routed to the killer pawn (10 default — no level-up at 100/level).
	TestTrue(TEXT("Killer received the XP reward"),
		FMath::IsNearlyEqual(Prog->Experience, Reward));
	TestEqual(TEXT("One kill does not level the character"), Prog->Level, 1);

	// Loot routed to the killer pawn's inventory.
	const FInventoryItem* Loot = Inv->Items.FindByPredicate(
		[&](const FInventoryItem& Item) { return Item.ItemId == FName(TEXT("HealthHerb")); });
	if (TestNotNull(TEXT("Loot item arrived in the killer inventory"), Loot))
	{
		TestEqual(TEXT("Loot stack count matches LootCount"), Loot->Count, 2);
	}

	// bDead guard: a second blow pays nothing (ADR-010b double-death fix).
	EnemyAsActor->TakeDamage(50.0f, FDamageEvent(), Controller, nullptr);
	TestTrue(TEXT("Blow after death pays no extra XP"),
		FMath::IsNearlyEqual(Prog->Experience, Reward));
	const FInventoryItem* Again = Inv->Items.FindByPredicate(
		[&](const FInventoryItem& Item) { return Item.ItemId == FName(TEXT("HealthHerb")); });
	if (TestNotNull(TEXT("Loot item still present exactly once"), Again))
	{
		TestEqual(TEXT("Blow after death grants no extra loot"), Again->Count, 2);
	}
	TestEqual(TEXT("No duplicate inventory entry appeared"), Inv->Items.Num(), 1);

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

/**
 *  XP/level data contract (Band 3 §12, the values the code HUD polls per tick
 *  in ADR-010e): non-positive gains are ignored, crossing ExperiencePerLevel
 *  levels up with the remainder kept, one large gain can cross several levels
 *  at once, and SetProgress clamps level >= 1 and XP >= 0 for loaded saves.
 *  The level-up itself broadcasts OnLevelUp; observing that would need a
 *  UObject with a UFUNCTION listener, so the value contract is asserted here.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOXpLevelUpAndClampTest,
	"SAOMMOnew.Progress.XpLevelUpAndClamp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOXpLevelUpAndClampTest::RunTest(const FString& Parameters)
{
	// The component comes from a spawned character (same pattern as the kill
	// reward test above) — pure arithmetic, but proven-in-this-file access.
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
	UProgressionComponent* Prog = Character->GetProgression();
	if (!TestNotNull(TEXT("Character owns a progression component"), Prog))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	TestEqual(TEXT("Fresh component starts at level 1"), Prog->Level, 1);
	TestTrue(TEXT("Fresh component starts with 0 XP"), FMath::IsNearlyZero(Prog->Experience));
	TestTrue(TEXT("Default threshold is positive"), Prog->ExperiencePerLevel > 0.0f);

	// 1) Non-positive gains are ignored (no free XP via zero/negative amounts).
	Prog->AddExperience(0.0f);
	Prog->AddExperience(-50.0f);
	TestEqual(TEXT("Zero/negative XP leaves the level alone"), Prog->Level, 1);
	TestTrue(TEXT("Zero/negative XP leaves XP unchanged"), FMath::IsNearlyZero(Prog->Experience));

	// 2) Crossing the threshold levels up and keeps the remainder.
	Prog->AddExperience(150.0f); // 100 consumed, 50 carried over.
	TestEqual(TEXT("Crossing the threshold levels up"), Prog->Level, 2);
	TestTrue(TEXT("Remainder above the threshold is kept"), FMath::IsNearlyEqual(Prog->Experience, 50.0f));

	// 3) One large gain can cross several levels at once (while-loop).
	Prog->AddExperience(300.0f); // 50 + 300 = 350 → three level-ups, 50 left.
	TestEqual(TEXT("A large gain resolves every crossed level"), Prog->Level, 5);
	TestTrue(TEXT("Remainder survives multiple level-ups"), FMath::IsNearlyEqual(Prog->Experience, 50.0f));

	// 4) SetProgress clamps (save/load path, ADR-011c).
	Prog->SetProgress(0, -10.0f);
	TestEqual(TEXT("SetProgress clamps level to >= 1"), Prog->Level, 1);
	TestTrue(TEXT("SetProgress clamps XP to >= 0"), FMath::IsNearlyZero(Prog->Experience));
	Prog->SetProgress(7, 42.5f);
	TestEqual(TEXT("SetProgress restores the saved level"), Prog->Level, 7);
	TestTrue(TEXT("SetProgress restores the saved XP"), FMath::IsNearlyEqual(Prog->Experience, 42.5f));

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

/**
 *  HUD contract (ADR-010e): the code HUD must be safe with no widget tree and
 *  no pawn — the controller can outlive its pawn across respawns, so SetStats
 *  and ToggleInventory may run before/without a constructed tree. The string
 *  formatting itself ("HP %.0f / %.0f", "Lv %d  XP %.0f", "[E] %s") sits
 *  behind TextBlock null-guards and only becomes observable with a real
 *  widget tree (CreateWidget needs a GameInstance = editor session), so the
 *  F5 visual check remains the acceptance for the rendered text.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOHudWidgetContractTest,
	"SAOMMOnew.HUD.HudWidgetContract",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOHudWidgetContractTest::RunTest(const FString& Parameters)
{
	UPlayerHudWidget* Hud = NewObject<UPlayerHudWidget>();
	if (!TestNotNull(TEXT("HUD widget created"), Hud))
	{
		return false;
	}

	TestFalse(TEXT("Inventory screen starts closed"), Hud->IsInventoryOpen());

	// No tree, no pawn: both calls must run without touching null members.
	Hud->SetStats(6.0f, 10.0f, 3, 42.5f, TEXT("ItemPickup_0"));
	Hud->SetStats(-1000.0f, 0.0f, -7, -1.0f, FString()); // degenerate inputs

	Hud->ToggleInventory();
	TestTrue(TEXT("Toggle opens the inventory screen"), Hud->IsInventoryOpen());
	Hud->ToggleInventory();
	TestFalse(TEXT("Toggle closes the inventory screen"), Hud->IsInventoryOpen());

	return true;
}

#endif // WITH_AUTOMATION_TESTS
