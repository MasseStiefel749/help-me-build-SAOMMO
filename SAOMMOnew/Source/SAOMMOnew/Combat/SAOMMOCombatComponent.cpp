// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOCombatComponent.h"
#include "SAOSword.h"
#include "SAOMMOInputFrameComponent.h"
#include "Engine/World.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"

USAOMMOCombatComponent::USAOMMOCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void USAOMMOCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InputFrame && GetOwner())
	{
		InputFrame = GetOwner()->FindComponentByClass<USAOMMOInputFrameComponent>();
	}
}

void USAOMMOCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!InputFrame)
	{
		if (GetOwner())
		{
			InputFrame = GetOwner()->FindComponentByClass<USAOMMOInputFrameComponent>();
		}
		if (!InputFrame)
		{
			return;
		}
	}

	const bool bAttack = InputFrame->CurrentFrame.bAttack;
	const float SwingSpeed = Sword ? Sword->GetSwingVelocity().Size() : 0.0f;

	// Rising edge of attack intent, or a fast swing, starts an armed window.
	const bool bRisingEdge = bAttack && !bPrevAttack;
	if (bRisingEdge || SwingSpeed >= SwingArmThreshold)
	{
		ArmTimer = ArmedDuration;
		if (!bArmed)
		{
			OnSwingStarted.Broadcast();
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
					if (!Target)
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

					const float Applied = Target->TakeDamage(Sword->Damage, FDamageEvent(),
						GetOwner() ? GetOwner()->GetInstigatorController() : nullptr, Sword);
					Sword->OnSwordHit.Broadcast(Target, Applied);
				}
			}
		}
	}
}
