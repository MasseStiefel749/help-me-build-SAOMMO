// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "Checkpoint.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "GameFramework/Controller.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Checkpoint flow (#19 Testinfrastruktur, Band 3 §11): entering the
 *  checkpoint volume as the player character must write the checkpoint pose
 *  (rotation + location, unit scale) into the owning MainPlayerController —
 *  the bookkeeping death respawns use. Runs in a bare test world; the overlap
 *  is dispatched through the public AActor entry point, exactly the way the
 *  engine dispatches it when a component overlap fires.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOCheckpointRespawnFlowTest,
	"SAOMMOnew.World.CheckpointRespawnFlow",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOCheckpointRespawnFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	const FRotator CheckpointRotation(0.0f, 45.0f, 0.0f);
	const FVector CheckpointLocation(1000.0f, 2000.0f, 50.0f);
	ACheckpoint* Checkpoint = World->SpawnActor<ACheckpoint>(CheckpointLocation, CheckpointRotation);
	APlayerCharacter* Character = World->SpawnActor<APlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AMainPlayerController* Controller = World->SpawnActor<AMainPlayerController>();

	const bool bSpawned = TestNotNull(TEXT("Checkpoint spawned"), Checkpoint)
		&& TestNotNull(TEXT("Player character spawned"), Character)
		&& TestNotNull(TEXT("MainPlayerController spawned"), Controller);

	if (bSpawned)
	{
		Controller->Possess(Character);
		TestTrue(TEXT("Character is possessed by the test controller"),
			Character->GetController() == Controller);

		AActor* CheckpointAsActor = Checkpoint; // public dispatch entry (engine does the same)
		CheckpointAsActor->NotifyActorBeginOverlap(Character);

		const FTransform Respawn = Controller->GetRespawnTransform();
		TestTrue(TEXT("Respawn location equals checkpoint location"),
			Respawn.GetLocation().Equals(Checkpoint->GetActorLocation(), 0.01f));
		TestTrue(TEXT("Respawn rotation equals checkpoint rotation"),
			FQuat(Respawn.GetRotation()).Equals(Checkpoint->GetActorQuat(), 1.0e-4f));
		TestTrue(TEXT("Respawn scale stays unit"),
			Respawn.GetScale3D().Equals(FVector::OneVector));
	}

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
