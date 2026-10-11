// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Boot contract: the project must boot directly into the playable L_StartingReach map
 *  with MainGameMode as the global default game mode. L_MainMenu remains an
 *  unfinished optional menu map. Check startup config, both map packages,
 *  and the resolvable native class.
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
	TestEqual(TEXT("GameDefaultMap points at L_StartingReach"),
		DefaultMap, FString(TEXT("/Game/Levels/StartingReach/L_StartingReach.L_StartingReach")));

	FString GameMode;
	GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"),
		TEXT("GlobalDefaultGameMode"), GameMode, GEngineIni);
	TestEqual(TEXT("GlobalDefaultGameMode is MainGameMode"),
		GameMode, FString(TEXT("/Script/SAOMMOnew.MainGameMode")));

	// 2) The menu and gameplay map packages exist on disk.
	TestTrue(TEXT("L_MainMenu package exists on disk"),
		FPackageName::DoesPackageExist(TEXT("/Game/Levels/L_MainMenu")));
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
