// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Boot contract (#19 Testinfrastruktur): the project must boot into
 *  L_StartingReach with MainGameMode as the global default game mode — the
 *  round-1 decision recorded in the night log. A regression here silently
 *  changes what players get when they press Play, so it is checked at three
 *  levels: config key, map package on disk, resolvable native class.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOBootDefaultMapAndGameModeTest,
	"SAOMMOnew.Boot.DefaultMapAndGameMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOBootDefaultMapAndGameModeTest::RunTest(const FString& Parameters)
{
	if (!GConfig)
	{
		AddError(TEXT("GConfig is not available in this context."));
		return false;
	}

	// 1) Merged config view (DefaultEngine.ini + engine defaults).
	FString DefaultMap;
	GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"),
		TEXT("GameDefaultMap"), DefaultMap, GEngineIni);
	TestTrue(TEXT("GameDefaultMap points at L_StartingReach"),
		DefaultMap.StartsWith(TEXT("/Game/Levels/StartingReach/L_StartingReach")));

	FString GameMode;
	GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"),
		TEXT("GlobalDefaultGameMode"), GameMode, GEngineIni);
	TestEqual(TEXT("GlobalDefaultGameMode is MainGameMode"),
		GameMode, FString(TEXT("/Script/SAOMMOnew.MainGameMode")));

	// 2) The map package actually exists on disk (catches a renamed level).
	TestTrue(TEXT("L_StartingReach package exists on disk"),
		FPackageName::DoesPackageExist(TEXT("/Game/Levels/StartingReach/L_StartingReach")));

	// 3) The native game mode class resolves (catches a renamed/moved class).
	UClass* GameModeClass = FindObject<UClass>(nullptr, TEXT("/Script/SAOMMOnew.MainGameMode"));
	if (!GameModeClass)
	{
		GameModeClass = LoadObject<UClass>(nullptr, TEXT("/Script/SAOMMOnew.MainGameMode"));
	}
	TestNotNull(TEXT("MainGameMode class resolves"), GameModeClass);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
