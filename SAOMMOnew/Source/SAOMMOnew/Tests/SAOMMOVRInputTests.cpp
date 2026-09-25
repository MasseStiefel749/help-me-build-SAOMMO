// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "VRCharacter.h"
#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UnrealType.h"

#if WITH_AUTOMATION_TESTS

namespace SAOMMOVRInputTest
{
	/** Reflection readers for the pawn's protected UPROPERTYs (backlog #23).
	 *  The test deliberately does not widen production access — Band 6 §19 keeps
	 *  the contract observable through the reflected properties instead. */
	inline UObject* GetObjectProp(const UObject* Obj, const TCHAR* PropName)
	{
		if (!Obj)
		{
			return nullptr;
		}
		if (FObjectProperty* Prop = FindFProperty<FObjectProperty>(Obj->GetClass(), PropName))
		{
			return Prop->GetObjectPropertyValue_InContainer(Obj);
		}
		return nullptr;
	}

	inline float GetFloatProp(const UObject* Obj, const TCHAR* PropName)
	{
		if (!Obj)
		{
			return -1.0f;
		}
		if (FFloatProperty* Prop = FindFProperty<FFloatProperty>(Obj->GetClass(), PropName))
		{
			return Prop->GetPropertyValue_InContainer(Obj);
		}
		return -1.0f;
	}

	/** How often an action is bound in the enhanced component. */
	inline int32 CountBindings(const UEnhancedInputComponent* Enhanced, const UInputAction* Action)
	{
		int32 Count = 0;
		if (Enhanced && Action)
		{
			for (const TUniquePtr<FEnhancedInputActionEventBinding>& Binding : Enhanced->GetActionEventBindings())
			{
				if (Binding && Binding->GetAction() == Action)
				{
					++Count;
				}
			}
		}
		return Count;
	}

	/** Whether the given trigger event is among the bindings for an action. */
	inline bool HasTriggerEvent(const UEnhancedInputComponent* Enhanced, const UInputAction* Action, ETriggerEvent Event)
	{
		if (Enhanced && Action)
		{
			for (const TUniquePtr<FEnhancedInputActionEventBinding>& Binding : Enhanced->GetActionEventBindings())
			{
				if (Binding && Binding->GetAction() == Action && Binding->GetTriggerEvent() == Event)
				{
					return true;
				}
			}
		}
		return false;
	}
}

/**
 *  VR desktop-fallback input contract (backlog #23, R14): AVRCharacter mirrors
 *  APlayerCharacter's enhanced-input setup so a non-XR keyboard/mouse session is
 *  playable (Band 2 §4 desktop fallback). The test pins, without an HMD:
 *
 *  - WalkSpeed/SprintSpeed (400/650) mirror the desktop pawn and land in the
 *    movement component on construction,
 *  - PawnClientRestart() (the engine possession path) yields a
 *    UEnhancedInputComponent — i.e. DefaultInput.ini's DefaultInputComponentClass
 *    is honoured, without which the Cast<> in SetupPlayerInputComponent would
 *    silently bind nothing,
 *  - all five actions are wired: IA_Move/IA_Look (Triggered), IA_Jump/IA_Attack
 *    (Started) bound once each, and the code-only Shift sprint action built by
 *    EnsureSprintMapping (action + mapping context present) bound three times
 *    (Started/Completed/Canceled).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOVRInputBindingsTest,
	"SAOMMOnew.Input.VRDesktopFallback",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOVRInputBindingsTest::RunTest(const FString& Parameters)
{
	// --- CDO contract: the desktop mirror (no world required) -----------------
	const AVRCharacter* VRDefault = GetDefault<AVRCharacter>();
	if (!TestNotNull(TEXT("VR pawn CDO exists"), VRDefault))
	{
		return false;
	}
	const APlayerCharacter* DesktopDefault = GetDefault<APlayerCharacter>();
	if (!TestNotNull(TEXT("Desktop pawn CDO exists"), DesktopDefault))
	{
		return false;
	}

	const float VRWalk = SAOMMOVRInputTest::GetFloatProp(VRDefault, TEXT("WalkSpeed"));
	const float VRSprint = SAOMMOVRInputTest::GetFloatProp(VRDefault, TEXT("SprintSpeed"));
	const float DesktopWalk = SAOMMOVRInputTest::GetFloatProp(DesktopDefault, TEXT("WalkSpeed"));
	const float DesktopSprint = SAOMMOVRInputTest::GetFloatProp(DesktopDefault, TEXT("SprintSpeed"));

	TestEqual(TEXT("VR WalkSpeed equals desktop pawn"), VRWalk, DesktopWalk);
	TestEqual(TEXT("VR SprintSpeed equals desktop pawn"), VRSprint, DesktopSprint);
	TestEqual(TEXT("VR WalkSpeed default is 400 cm/s"), VRWalk, 400.0f);
	TestEqual(TEXT("VR SprintSpeed default is 650 cm/s"), VRSprint, 650.0f);

	// --- Possessed pawn → engine input setup ----------------------------------
	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	AVRCharacter* VR = World->SpawnActor<AVRCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("VR pawn spawned"), VR))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Construction applies WalkSpeed to the movement component (VRCharacter ctor).
	if (UCharacterMovementComponent* Movement = VR->GetCharacterMovement())
	{
		TestEqual(TEXT("Movement component got WalkSpeed on construction"),
			Movement->MaxWalkSpeed, 400.0f);
	}
	else
	{
		AddError(TEXT("VR pawn has no character movement component"));
	}

	APlayerController* PC = World->SpawnActor<APlayerController>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Player controller spawned"), PC))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// Bare test world: no camera manager to manage a view target for, and no
	// ULocalPlayer/NetDriver — APlayerController::IsLocalController()
	// (PlayerController.cpp:332) would classify our controller as *not* local
	// (its fast path only flips once a local player is assigned), which makes
	// PawnClientRestart() skip the input setup entirely. A real session gets
	// this flag from GameModeBase when the controller is created.
	PC->SetAsLocalPlayerController();
	PC->bAutoManageActiveCameraTarget = false;
	PC->Possess(VR);

	// Diagnostics (surfaced as Info events in the test report on failure):
	// pin down which PawnClientRestart guard failed, if any.
	AddInfo(FString::Printf(TEXT("diag-after-possess: controller=%s playerState=%s isLocal=%d authority=%d netMode=%d pcNetMode=%d pcRole=%d pcRemoteRole=%d"),
		*GetNameSafe(VR->GetController()),
		PC->PlayerState ? TEXT("set") : TEXT("null"),
		PC->IsLocalController() ? 1 : 0,
		PC->HasAuthority() ? 1 : 0,
		static_cast<int32>(World->GetNetMode()),
		static_cast<int32>(PC->GetNetMode()),
		static_cast<int32>(PC->GetLocalRole()),
		static_cast<int32>(PC->GetRemoteRole())));

	// Public engine hook (Pawn.h): creates the player input component from
	// DefaultInput.ini's DefaultInputComponentClass and runs our override.
	VR->PawnClientRestart();

	AddInfo(FString::Printf(TEXT("diag-after-restart: controller=%s inputComp=%s sprintAction=%s"),
		*GetNameSafe(VR->GetController()),
		VR->InputComponent ? TEXT("set") : TEXT("null"),
		SAOMMOVRInputTest::GetObjectProp(VR, TEXT("SprintAction")) ? TEXT("set") : TEXT("null")));

	if (!TestNotNull(TEXT("Input component created"), VR->InputComponent.Get()))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(VR->InputComponent);
	if (!TestNotNull(TEXT("Input component honours DefaultInputComponentClass (UEnhancedInputComponent)"), Enhanced))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	// --- Asset-backed actions: loaded and bound once each ---------------------
	using namespace SAOMMOVRInputTest;

	const UInputAction* Move = Cast<UInputAction>(GetObjectProp(VR, TEXT("MoveAction")));
	const UInputAction* Look = Cast<UInputAction>(GetObjectProp(VR, TEXT("LookAction")));
	const UInputAction* Jump = Cast<UInputAction>(GetObjectProp(VR, TEXT("JumpAction")));
	const UInputAction* Attack = Cast<UInputAction>(GetObjectProp(VR, TEXT("AttackAction")));

	TestNotNull(TEXT("IA_Move asset loaded"), Move);
	TestNotNull(TEXT("IA_Look asset loaded"), Look);
	TestNotNull(TEXT("IA_Jump asset loaded"), Jump);
	TestNotNull(TEXT("IA_Attack asset loaded"), Attack);

	TestEqual(TEXT("IA_Move bound once"), CountBindings(Enhanced, Move), 1);
	TestEqual(TEXT("IA_Look bound once"), CountBindings(Enhanced, Look), 1);
	TestEqual(TEXT("IA_Jump bound once"), CountBindings(Enhanced, Jump), 1);
	TestEqual(TEXT("IA_Attack bound once"), CountBindings(Enhanced, Attack), 1);

	// R14 trigger-event contract: same events as APlayerCharacter.
	TestTrue(TEXT("IA_Move uses Triggered"), HasTriggerEvent(Enhanced, Move, ETriggerEvent::Triggered));
	TestTrue(TEXT("IA_Look uses Triggered"), HasTriggerEvent(Enhanced, Look, ETriggerEvent::Triggered));
	TestTrue(TEXT("IA_Jump uses Started"), HasTriggerEvent(Enhanced, Jump, ETriggerEvent::Started));
	TestTrue(TEXT("IA_Attack uses Started"), HasTriggerEvent(Enhanced, Attack, ETriggerEvent::Started));

	// --- Code-only Shift sprint (EnsureSprintMapping ran inside setup) --------
	const UInputAction* Sprint = Cast<UInputAction>(GetObjectProp(VR, TEXT("SprintAction")));
	if (TestNotNull(TEXT("SprintAction built by EnsureSprintMapping"), Sprint))
	{
		TestEqual(TEXT("SprintAction bound three times"), CountBindings(Enhanced, Sprint), 3);
		TestTrue(TEXT("Sprint uses Started"), HasTriggerEvent(Enhanced, Sprint, ETriggerEvent::Started));
		TestTrue(TEXT("Sprint uses Completed"), HasTriggerEvent(Enhanced, Sprint, ETriggerEvent::Completed));
		TestTrue(TEXT("Sprint uses Canceled"), HasTriggerEvent(Enhanced, Sprint, ETriggerEvent::Canceled));
	}
	TestNotNull(TEXT("SprintMapping (Shift IMC) built"), GetObjectProp(VR, TEXT("SprintMapping")));

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
