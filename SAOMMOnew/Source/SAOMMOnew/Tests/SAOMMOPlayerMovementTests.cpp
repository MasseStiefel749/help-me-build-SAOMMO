// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "InputFrameComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Desktop movement contract (S04-P01): W/S input through OnMove must reach
 *  the character movement as pending input, and the input-frame recorder must
 *  observe the same vector. Runs synchronously — no tick required, because
 *  AddMovementInput only accumulates the pending vector here.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOPlayerMovementInputTest,
	"SAOMMOnew.Desktop.PlayerMovementInput",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOPlayerMovementInputTest::RunTest(const FString& Parameters)
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

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!TestNotNull(TEXT("Character owns a movement component"), Movement))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Fresh character: no pending input. NOTE: GetPendingInputVector (accumulated,
	// not yet consumed) is the right accessor here — GetLastInputVector only
	// reflects the last consumed tick.
	TestTrue(TEXT("Fresh character has no pending input"),
		Movement->GetPendingInputVector().IsNearlyZero());

	// Forward input (W): pending vector must point forward (+X at yaw 0).
	Character->OnMove(FInputActionValue(FVector2D(0.0f, 1.0f)));
	const FVector ForwardInput = Movement->GetPendingInputVector();
	TestTrue(TEXT("Forward input produces forward pending input"), ForwardInput.X > 0.5f);

	// The input-frame recorder observes the same vector (Band 2 §4 path intact).
	if (TestNotNull(TEXT("Character owns an input frame recorder"), Character->GetInputFrame()))
	{
		const FVector2D Recorded = Character->GetInputFrame()->CurrentFrame.Move;
		TestTrue(TEXT("Recorder observed the forward vector"),
			FMath::IsNearlyEqual(Recorded.X, 0.0f) && FMath::IsNearlyEqual(Recorded.Y, 1.0f));
	}

	// Look input runs without crashing (rotation itself applies on tick).
	Character->OnLook(FInputActionValue(FVector2D(10.0f, -5.0f)));
	TestTrue(TEXT("Look input handled without crash"), true);

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

/**
 *  Diagonal movement contract (S04-P01 edge case): a diagonal W+D press must
 *  reach both axes (forward + right at yaw 0). Engine-side acceleration
 *  clamping happens later in CharacterMovement; this guards our mapping math.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOPlayerMovementDiagonalTest,
	"SAOMMOnew.Desktop.PlayerMovementDiagonal",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOPlayerMovementDiagonalTest::RunTest(const FString& Parameters)
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

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!TestNotNull(TEXT("Character owns a movement component"), Movement))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	Character->OnMove(FInputActionValue(FVector2D(1.0f, 1.0f)));
	const FVector DiagonalInput = Movement->GetPendingInputVector();
	TestTrue(TEXT("Diagonal input drives the forward axis"), DiagonalInput.X > 0.5f);
	TestTrue(TEXT("Diagonal input drives the strafe axis"), DiagonalInput.Y > 0.5f);

	if (TestNotNull(TEXT("Character owns an input frame recorder"), Character->GetInputFrame()))
	{
		const FVector2D Recorded = Character->GetInputFrame()->CurrentFrame.Move;
		TestTrue(TEXT("Recorder observed the diagonal vector"),
			FMath::IsNearlyEqual(Recorded.X, 1.0f) && FMath::IsNearlyEqual(Recorded.Y, 1.0f));
	}

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
