// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "ProgressionComponent.h"
#include "InventoryComponent.h"
#include "ItemTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = ApproachSpeed;
		// Smooth turning toward movement input (replaces the old instant
		// snap in MoveTowardTarget).
		GetCharacterMovement()->bOrientRotationToMovement = true;
		// REQUIRED: CharacterMovement skips simulation for controller-less
		// pawns unless this is set — without it AddMovementInput does
		// nothing and enemies stand frozen.
		GetCharacterMovement()->bRunPhysicsWithNoController = true;
	}

	// Visible body: Quinn mesh so enemies read differently from the player.
	// Guarded: a missing asset simply leaves the collision capsule (status
	// quo), it can never break the spawn.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
	if (BodyMesh.Succeeded() && GetMesh())
	{
		GetMesh()->SetSkeletalMesh(BodyMesh.Object);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	// Same skeleton family as the player rig, so the Unarmed anim applies.
	static ConstructorHelpers::FClassFinder<UAnimInstance> BodyAnim(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (BodyAnim.Succeeded() && GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(BodyAnim.Class);
	}

	// Front-fall death (same skeleton family). Guarded; without it the
	// corpse simply stands through its lifespan.
	static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathObj(
		TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Front_01"));
	if (DeathObj.Succeeded())
	{
		DeathAnim = DeathObj.Object;
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	State = EEnemyState::Idle;

	// Apply the (possibly Editor-tuned) approach speed; the constructor only
	// sees the C++ default.
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = ApproachSpeed;
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Corpses keep ticking (mesh anim + lifespan) but the FSM is done.
	if (bDead)
	{
		return;
	}

	UpdateState(DeltaTime);
}

void AEnemy::UpdateState(float DeltaTime)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Acquire / validate the target once per tick.
	if (!TargetPawn.IsValid())
	{
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			TargetPawn = Player;
		}
	}

	switch (State)
	{
	case EEnemyState::Idle:
		if (TargetPawn.IsValid())
		{
			const float Dist = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
			if (Dist <= DetectRange)
			{
				State = EEnemyState::Approach;
			}
		}
		break;

	case EEnemyState::Approach:
		if (!TargetPawn.IsValid())
		{
			State = EEnemyState::Idle;
			break;
		}
		{
			const float Dist = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
			if (Dist <= AttackRange)
			{
				State = EEnemyState::Attack;
			}
			else if (Dist > DetectRange * 2.0f)
			{
				// Leash: outran the encounter, drop back to idle.
				TargetPawn = nullptr;
				State = EEnemyState::Idle;
			}
			else
			{
				MoveTowardTarget(DeltaTime);
			}
		}
		break;

	case EEnemyState::Attack:
		if (!TargetPawn.IsValid())
		{
			State = EEnemyState::Idle;
			break;
		}
		PerformAttack();
		RecoverRemaining = RecoverTime;
		State = EEnemyState::Recover;
		break;

	case EEnemyState::Recover:
		RecoverRemaining -= DeltaTime;
		if (RecoverRemaining <= 0.0f)
		{
			State = EEnemyState::Approach;
		}
		break;

	default:
		break;
	}
}

void AEnemy::MoveTowardTarget(float DeltaTime)
{
	if (!TargetPawn.IsValid())
	{
		return;
	}

	// Steer through CharacterMovement (no NavMesh needed for direct input
	// steering). This gives real velocity, so the locomotion anim blends,
	// collision slides, and turning smooths via bOrientRotationToMovement.
	// (Was: SetActorLocation teleport, which left velocity at zero = the
	// anim graph always saw idle, plus instant rotation snaps.)
	(void)DeltaTime;
	const FVector Direction = (TargetPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	AddMovementInput(Direction, 1.0f);
}

void AEnemy::PerformAttack()
{
	if (!TargetPawn.IsValid() || !bAttackReady)
	{
		return;
	}

	const float Dist = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
	if (Dist <= AttackRange)
	{
		TargetPawn->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
		bAttackReady = false;
		// Cooldown tied to the recover period (was SetTimerForNextTick = no cooldown).
		if (UWorld* World = GetWorld())
		{
			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, [this]()
			{
				bAttackReady = true;
			}, FMath::Max(0.1f, RecoverTime), false);
		}
		else
		{
			bAttackReady = true;
		}
	}
}

float AEnemy::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead || CurrentHealth <= 0.0f || Damage <= 0.0f)
	{
		return 0.0f;
	}

	if (EventInstigator)
	{
		Killer = EventInstigator;
	}
	else if (DamageCauser)
	{
		// Melee path passes the sword as causer with a null controller;
		// credit its instigator so XP/loot still route to the killer.
		Killer = DamageCauser->GetInstigatorController();
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AEnemy::Die()
{
	if (bDead)
	{
		return;
	}
	bDead = true;
	State = EEnemyState::Idle;

	// Corpse handling: no collision (passes through cleanly), death fall
	// animation (mesh keeps ticking; Tick() skips the FSM once bDead).
	// Lifespan set below destroys the actor after the fall.
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (DeathAnim && GetMesh())
	{
		GetMesh()->PlayAnimation(DeathAnim, false);
	}

	// Fight->Loot->Improve: grant XP + loot to the killer's components.
	if (APawn* KillerPawn = Killer.IsValid() ? Killer->GetPawn() : nullptr)
	{
		if (UProgressionComponent* Prog = KillerPawn->FindComponentByClass<UProgressionComponent>())
		{
			Prog->AddExperience(XPReward);
		}
		if (!LootItemId.IsNone() && LootCount > 0)
		{
			if (UInventoryComponent* Inv = KillerPawn->FindComponentByClass<UInventoryComponent>())
			{
				FInventoryItem Loot;
				Loot.ItemId = LootItemId;
				Loot.Count = LootCount;
				Inv->AddItem(Loot);
			}
		}
	}

	OnDied.Broadcast();

	if (GetWorld())
	{
		SetLifeSpan(2.0f);
	}
	else
	{
		Destroy();
	}
}

void AEnemy::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (bDead || CurrentHealth <= 0.0f || Damage <= 0.0f)
	{
		return;
	}

	if (DamageCauser)
	{
		Killer = DamageCauser->GetInstigatorController();
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);

	if (DamageImpulse.SizeSquared() > KINDA_SMALL_NUMBER && GetCharacterMovement())
	{
		GetCharacterMovement()->AddImpulse(DamageImpulse, true);
	}

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void AEnemy::HandleDeath()
{
	Die();
}

void AEnemy::ApplyHealing(float Healing, AActor* Healer)
{
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Healing);
}

void AEnemy::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// First prototype: simply become aware of the player when danger is nearby.
	if (APawn* Player = Cast<APawn>(DangerSource))
	{
		TargetPawn = Player;
		if (State == EEnemyState::Idle)
		{
			State = EEnemyState::Approach;
		}
	}
}
