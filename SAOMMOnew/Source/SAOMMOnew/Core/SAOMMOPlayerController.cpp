// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "SAOMMOCharacter.h"
#include "SAOMMOGameMode.h"
#include "SAOMMOHudWidget.h"

ASAOMMOPlayerController::ASAOMMOPlayerController()
{
	// Code-only HUD works with zero Editor setup; a Blueprint child can
	// override HudWidgetClass with a styled widget later.
	HudWidgetClass = USAOMMOHudWidget::StaticClass();
}

void ASAOMMOPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Code-only HUD for local players; polls the pawn so it survives respawn.
	if (IsLocalPlayerController() && !HudWidget && *HudWidgetClass)
	{
		HudWidget = CreateWidget<USAOMMOHudWidget>(this, HudWidgetClass);
		if (HudWidget)
		{
			HudWidget->AddToViewport();
		}
	}
	// Cache current transform as default respawn so death doesn't drop to origin.
	if (RespawnTransform.Equals(FTransform::Identity))
	{
		if (APawn* P = GetPawn())
		{
			RespawnTransform = P->GetActorTransform();
		}
		else if (PlayerCameraManager)
		{
			// AController hides GetActorTransform() (private in 5.8), so fall back
			// to the camera transform. OnPossess() will overwrite this once a pawn exists.
			RespawnTransform = FTransform(GetControlRotation(), PlayerCameraManager->GetCameraLocation(), FVector::OneVector);
		}
		RespawnTransform.SetScale3D(FVector::OneVector);
	}
}

void ASAOMMOPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				for (UInputMappingContext* Context : DefaultMappingContexts)
				{
					if (Context)
					{
						Subsystem->AddMappingContext(Context, 0);
					}
				}
			}
		}
	}
}

void ASAOMMOPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		InPawn->OnDestroyed.AddDynamic(this, &ASAOMMOPlayerController::OnPawnDestroyed);
		// Update respawn to last possessed location if still identity (first spawn / checkpoint-less run).
		if (RespawnTransform.Equals(FTransform::Identity) || RespawnTransform.GetLocation().IsNearlyZero())
		{
			RespawnTransform = InPawn->GetActorTransform();
			RespawnTransform.SetScale3D(FVector::OneVector);
		}
	}
}

void ASAOMMOPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	RespawnTransform = NewRespawn;
}

void ASAOMMOPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	TSubclassOf<ASAOMMOCharacter> SpawnClass = CharacterClass;

	if (!SpawnClass)
	{
		if (const ASAOMMOGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASAOMMOGameMode>() : nullptr)
		{
			SpawnClass = GameMode->DefaultCharacterClass;
		}
	}

	if (!SpawnClass)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (ASAOMMOCharacter* Respawned = World->SpawnActor<ASAOMMOCharacter>(SpawnClass, RespawnTransform))
		{
			Possess(Respawned);
		}
	}
}
