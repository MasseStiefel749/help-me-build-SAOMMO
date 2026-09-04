// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOEnemy.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"

ASAOEnemy::ASAOEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = ApproachSpeed;
	}
}

void ASAOEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	State = ESAOEnemyState::Idle;
}

void ASAOEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateState(DeltaTime);
}

void ASAOEnemy::UpdateState(float DeltaTime)
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
	case ESAOEnemyState::Idle:
		if (TargetPawn.IsValid())
		{
			const float Dist = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
			if (Dist <= DetectRange)
			{
				State = ESAOEnemyState::Approach;
			}
		}
		break;

	case ESAOEnemyState::Approach:
		if (!TargetPawn.IsValid())
		{
			State = ESAOEnemyState::Idle;
			break;
		}
		{
			const float Dist = FVector::Dist(GetActorLocation(), TargetPawn->GetActorLocation());
			if (Dist <= AttackRange)
			{
				State = ESAOEnemyState::Attack;
			}
			else
			{
				MoveTowardTarget(DeltaTime);
			}
		}
		break;

	case ESAOEnemyState::Attack:
		if (!TargetPawn.IsValid())
		{
			State = ESAOEnemyState::Idle;
			break;
		}
		PerformAttack();
		RecoverRemaining = RecoverTime;
		State = ESAOEnemyState::Recover;
		break;

	case ESAOEnemyState::Recover:
		RecoverRemaining -= DeltaTime;
		if (RecoverRemaining <= 0.0f)
		{
			State = ESAOEnemyState::Approach;
		}
		break;

	default:
		break;
	}
}

void ASAOEnemy::MoveTowardTarget(float DeltaTime)
{
	if (!TargetPawn.IsValid())
	{
		return;
	}

	const FVector Direction = (TargetPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	const FVector NewLocation = GetActorLocation() + Direction * ApproachSpeed * DeltaTime;

	SetActorLocation(NewLocation, true);
	SetActorRotation(Direction.Rotation());
}

void ASAOEnemy::PerformAttack()
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
		// Simple cooldown tied to the recover period.
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			bAttackReady = true;
		});
	}
}

float ASAOEnemy::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth -= Damage;

	if (CurrentHealth <= 0.0f)
	{
		Die();
		return Damage;
	}

	return Damage;
}

void ASAOEnemy::Die()
{
	State = ESAOEnemyState::Idle;
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

void ASAOEnemy::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (CurrentHealth <= 0.0f)
	{
		return;
	}

	CurrentHealth -= Damage;

	if (DamageImpulse.SizeSquared() > KINDA_SMALL_NUMBER && GetCharacterMovement())
	{
		GetCharacterMovement()->AddImpulse(DamageImpulse, true);
	}

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ASAOEnemy::HandleDeath()
{
	Die();
}

void ASAOEnemy::ApplyHealing(float Healing, AActor* Healer)
{
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Healing);
}

void ASAOEnemy::NotifyDanger(const FVector& DangerLocation, AActor* DangerSource)
{
	// First prototype: simply become aware of the player when danger is nearby.
	if (APawn* Player = Cast<APawn>(DangerSource))
	{
		TargetPawn = Player;
		if (State == ESAOEnemyState::Idle)
		{
			State = ESAOEnemyState::Approach;
		}
	}
}
