// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatComponent.h"
#include "Sword.h"
#include "CombatInterfaces.h"
#include "FloatingCombatText.h"
#include "InputFrameComponent.h"
#include "Engine/World.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InputFrame && GetOwner())
	{
		InputFrame = GetOwner()->FindComponentByClass<UInputFrameComponent>();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InputFrame)
	{
		if (GetOwner())
		{
			InputFrame = GetOwner()->FindComponentByClass<UInputFrameComponent>();
		}
		if (!InputFrame)
		{
			return;
		}
	}

	const bool bAttack = InputFrame->CurrentFrame.bAttack;
	const float SwingSpeed = Sword ? Sword->GetSwingVelocity().Size() : 0.0f;

	// Rising edge of attack intent, or a fast swing, starts an armed window.
	// Velocity arming ONLY counts on motion devices: on desktop the sword is
	// glued to the body, so walking (400 cm/s) would arm it permanently and
	// every step would attack. Desktop attacks come from the LMB edge only.
	const bool bFromMotionDevice = InputFrame
		&& (InputFrame->CurrentFrame.SourceDevice == EInputDevice::VRController
			|| InputFrame->CurrentFrame.SourceDevice == EInputDevice::BudgetVR);
	const bool bRisingEdge = bAttack && !bPrevAttack;
	if (bRisingEdge || (bFromMotionDevice && SwingSpeed >= SwingArmThreshold))
	{
		ArmTimer = ArmedDuration;
		if (!bArmed)
		{
			OnSwingStarted.Broadcast();
			// Start the visible swing from the sword's current pose.
			SwingAlpha = 0.0f;
			if (Sword && Sword->GetRootComponent())
			{
				SwingStartRotation = Sword->GetRootComponent()->GetRelativeRotation();
			}
		}
	}
	bPrevAttack = bAttack;

	// Consume the edge-triggered attack flag so it is handled exactly once.
	InputFrame->CurrentFrame.bAttack = false;

	ArmTimer = FMath::Max(0.0f, ArmTimer - DeltaTime);
	bArmed = ArmTimer > 0.0f;

	if (Sword)
	{
		Sword->SetHitEnabled(bArmed);

		// Visible swing: slash across the roll plane (blade lies along the
		// grip Y axis, so pitching would roll it in place invisibly). Sweeps
		// 120 deg from rest pose, then restores it. The damage query is
		// location-based, so rotating is purely cosmetic.
		if (bArmed && ArmedDuration > KINDA_SMALL_NUMBER && Sword->GetRootComponent())
		{
			SwingAlpha = FMath::Clamp(SwingAlpha + DeltaTime / ArmedDuration, 0.0f, 1.0f);
			const float Roll = FMath::Lerp(0.0f, -120.0f, SwingAlpha);
			Sword->GetRootComponent()->SetRelativeRotation(
				FRotator(SwingStartRotation.Pitch, SwingStartRotation.Yaw, SwingStartRotation.Roll + Roll));
		}
		else if (bWasArmed && Sword->GetRootComponent())
		{
			Sword->GetRootComponent()->SetRelativeRotation(SwingStartRotation);
		}
		bWasArmed = bArmed;

		// Active melee query: while armed, apply the sword's damage to any pawn
		// within reach. Active detection (rather than relying on overlap begin
		// events) ensures a hit lands even if the target was already in range
		// before the swing started.
		if (bArmed && GetWorld())
		{
			const FVector Origin = Sword->GetActorLocation();
			TArray<FOverlapResult> Hits;
			FCollisionQueryParams Params;
			if (GetOwner())
			{
				Params.AddIgnoredActor(GetOwner());
			}
			Params.AddIgnoredActor(Sword);

			if (GetWorld()->OverlapMultiByChannel(Hits, Origin, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(HitReach), Params))
			{
				const float Now = GetWorld()->GetTimeSeconds();
				for (const FOverlapResult& Hit : Hits)
				{
					AActor* Target = Hit.GetActor();
					if (!Target || Target == GetOwner())
					{
						continue;
					}
					// Never hit the wielder recorded on the sword either.
					if (Sword->GetOwnerActor() && Target == Sword->GetOwnerActor())
					{
						continue;
					}

					if (float* Last = LastHitTime.Find(Target))
					{
						if (Now - *Last < Sword->PerTargetCooldown)
						{
							continue;
						}
					}
					LastHitTime.Add(Target, Now);

					const FVector HitLocation = Hit.GetComponent()
						? Hit.GetComponent()->GetComponentLocation()
						: Target->GetActorLocation();
					float Applied = 0.0f;
					// Prefer the shared damageable contract so enemy logic
					// (double-death guard, killer credit) runs in one place.
					if (Target->Implements<UDamageable>())
					{
						if (IDamageable* Damageable = Cast<IDamageable>(Target))
						{
							Damageable->ApplyDamage(Sword->Damage, Sword,
								HitLocation, FVector::ZeroVector);
							// ApplyDamage has no return; report configured damage.
							Applied = Sword->Damage;
						}
					}
					else
					{
						Applied = Target->TakeDamage(Sword->Damage, FDamageEvent(),
							GetOwner() ? GetOwner()->GetInstigatorController() : nullptr, Sword);
					}
					Sword->OnSwordHit.Broadcast(Target, Applied);
					OnHit.Broadcast(Target, Applied, HitLocation);
					// Floating damage number above the hit (SAO-style feedback).
					if (UWorld* World = GetWorld())
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride =
							ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						if (AFloatingCombatText* Number = World->SpawnActor<AFloatingCombatText>(
							AFloatingCombatText::StaticClass(),
							HitLocation + FVector(0.0f, 0.0f, 30.0f),
							FRotator::ZeroRotator, SpawnParams))
						{
							Number->Configure(Applied, FLinearColor(1.0f, 0.85f, 0.2f));
						}
					}
					// Playtest feedback: combat is otherwise invisible.
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
							FString::Printf(TEXT("Hit %s (%.0f)"), *Target->GetName(), Applied));
					}
				}
			}
		}
	}
}
