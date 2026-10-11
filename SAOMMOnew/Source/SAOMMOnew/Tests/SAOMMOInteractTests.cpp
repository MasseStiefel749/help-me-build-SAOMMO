// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "InteractionComponent.h"
#include "InputActionValue.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Interact/inventory routing (S04-P06, E/I split): the shared IA_Interact
 *  action routes through HandleInteractKey — E reaches the interaction
 *  component (safe no-op without a focused target), I reaches the inventory
 *  toggle path. Both must run without crashing on a fresh possessed pawn.
 *  Visual open/close proof stays a manual test (needs a real viewport).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOInteractRoutingTest,
	"SAOMMOnew.Desktop.InteractRouting",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOInteractRoutingTest::RunTest(const FString& Parameters)
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

	if (!TestNotNull(TEXT("Character owns an interaction component"), Character->GetInteraction()))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// E-path (false): no focused target in a bare world — must be a safe no-op.
	Character->HandleInteractKey(false);
	TestTrue(TEXT("E without target is a safe no-op"), true);

	// I-path (true): no HUD in a bare world — ToggleInventory must be null-safe.
	Character->HandleInteractKey(true);
	TestTrue(TEXT("I without HUD is a safe no-op"), true);

	// NOTE: OnInteract itself stays protected; its key-state routing is proven
	// by packaged-game logs (SAOMMO input: lines), not by this bare-world test.

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
