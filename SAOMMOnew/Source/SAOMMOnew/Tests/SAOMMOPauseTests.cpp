// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOnew.h"
#include "SAOMMOTestWorld.h"
#include "MainPlayerController.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Pause menu contract (Esc): toggling must never crash, even without a
 *  viewport or game mode (bare automation world). Full state proof —
 *  paused flag, visible overlay, input-mode switch — needs a real viewport
 *  and stays a manual test (NEEDS MANUAL TEST: Esc in a running game).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOPauseMenuSafetyTest,
	"SAOMMOnew.Desktop.PauseMenuSafety",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOPauseMenuSafetyTest::RunTest(const FString& Parameters)
{
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	AMainPlayerController* Controller = World->SpawnActor<AMainPlayerController>();
	if (!TestNotNull(TEXT("MainPlayerController spawned"), Controller))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Toggle twice in a viewport-less world: must complete without crashing.
	// (Paused flag + overlay visibility are asserted in the manual Esc test.)
	Controller->TogglePauseMenu();
	Controller->TogglePauseMenu();
	TestTrue(TEXT("Pause toggle is safe without viewport"), true);

	UE_LOG(LogGame, Display, TEXT("SAOMMO test: pause menu safety path completed"));

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
