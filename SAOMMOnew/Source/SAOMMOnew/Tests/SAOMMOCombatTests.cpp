// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "SAOMMOTestWorld.h"
#include "Enemy.h"
#include "Sword.h"
#include "CombatInterfaces.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter.h"
#include "MainPlayerController.h"
#include "InputFrameComponent.h"
#include "InputActionValue.h"

#if WITH_AUTOMATION_TESTS

/**
 *  Sword damage formula (#19 Testinfrastruktur, Band 2 §8/§10): the sword's
 *  configured Damage flows unchanged into IDamageable::ApplyDamage, health
 *  subtracts with a floor of 0, and the killing blow runs Die() — guarded
 *  against a second death. Runs in a bare test world; enemy health is set
 *  explicitly because BeginPlay (which copies MaxHealth) does not run there.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOSwordDamageFormulaTest,
	"SAOMMOnew.Combat.SwordDamageFormula",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOSwordDamageFormulaTest::RunTest(const FString& Parameters)
{
	// The weapon's configured value (CDO; editor-tuned values live on
	// Blueprints — the native default must stay positive so a hit can never
	// be zero-damage).
	const ASword* SwordCDO = GetDefault<ASword>();
	if (!TestNotNull(TEXT("Sword CDO exists"), SwordCDO))
	{
		return false;
	}
	TestTrue(TEXT("Sword default damage is positive"), SwordCDO->Damage > 0.0f);
	const float SwordDamage = SwordCDO->Damage;

	UWorld* World = SAOMMOTest::CreateTestWorld();
	if (!TestNotNull(TEXT("Test world created"), World))
	{
		return false;
	}

	AEnemy* Enemy = World->SpawnActor<AEnemy>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Enemy spawned"), Enemy))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}
	IDamageable* Damageable = Cast<IDamageable>(Enemy);
	if (!TestNotNull(TEXT("Enemy implements IDamageable"), Damageable))
	{
		SAOMMOTest::DestroyTestWorld(World);
		return false;
	}

	Enemy->CurrentHealth = 3.0f; // BeginPlay does not run in a bare world.

	// 1) Configured sword damage applies unchanged (3 - 1 = 2 by default).
	Damageable->ApplyDamage(SwordDamage, nullptr, FVector::ZeroVector, FVector::ZeroVector);
	TestTrue(TEXT("Health reduced by exactly the sword damage"),
		FMath::IsNearlyEqual(Enemy->CurrentHealth, 3.0f - SwordDamage));

	// 2) Guards: zero damage is ignored (no free self-heal via negatives).
	Damageable->ApplyDamage(0.0f, nullptr, FVector::ZeroVector, FVector::ZeroVector);
	TestTrue(TEXT("Zero damage leaves health unchanged"),
		FMath::IsNearlyEqual(Enemy->CurrentHealth, 3.0f - SwordDamage));

	// 3) Floor at 0 + death: overkill clamps and runs Die() — first visible
	//    side effect is the corpse capsule no longer blocking.
	Enemy->CurrentHealth = 0.5f;
	Damageable->ApplyDamage(SwordDamage + 10.0f, nullptr, FVector::ZeroVector, FVector::ZeroVector);
	TestTrue(TEXT("Health clamps at 0"), Enemy->CurrentHealth == 0.0f);
	if (UCapsuleComponent* Capsule = Enemy->GetCapsuleComponent())
	{
		TestTrue(TEXT("Die() ran: corpse capsule no longer blocks"),
			Capsule->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
	}
	else
	{
		AddError(TEXT("Enemy has no capsule component."));
	}

	// 4) Double-death guard: a second killing blow changes nothing.
	Damageable->ApplyDamage(100.0f, nullptr, FVector::ZeroVector, FVector::ZeroVector);
	TestTrue(TEXT("Blow after death changes nothing"), Enemy->CurrentHealth == 0.0f);

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

/**
 *  Attack input contract (S04-P05 UI focus): OnAttack arms the swing through
 *  the input frame, and a missing/closed inventory never suppresses it
 *  (IsInventoryOpen is false with no HUD). The open-inventory suppression
 *  itself needs a live HUD and stays a manual test: open with I, press LMB,
 *  no swing may happen.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAOMMOAttackInputArmsTest,
	"SAOMMOnew.Combat.AttackInputArms",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSAOMMOAttackInputArmsTest::RunTest(const FString& Parameters)
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

	// Safe default: no HUD yet means no open inventory, attacks arm normally.
	TestFalse(TEXT("Inventory reports closed with no HUD"), Controller->IsInventoryOpen());

	Character->OnAttack(FInputActionValue());
	if (TestNotNull(TEXT("Character owns an input frame recorder"), Character->GetInputFrame()))
	{
		TestTrue(TEXT("Attack input arms the swing"), Character->GetInputFrame()->CurrentFrame.bAttack);
	}

	SAOMMOTest::DestroyTestWorld(World);
	return true;
}

#endif // WITH_AUTOMATION_TESTS
