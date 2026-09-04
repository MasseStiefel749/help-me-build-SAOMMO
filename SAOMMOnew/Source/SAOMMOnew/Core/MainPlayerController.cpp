// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "PlayerCharacter.h"
#include "MainGameMode.h"
#include "PlayerHudWidget.h"
#include "PlayerSaveGame.h"
#include "InventoryComponent.h"
#include "ProgressionComponent.h"

AMainPlayerController::AMainPlayerController()
{
	// Code-only HUD works with zero Editor setup; a Blueprint child can
	// override HudWidgetClass with a styled widget later.
	HudWidgetClass = UPlayerHudWidget::StaticClass();
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// Code-only HUD for local players; polls the pawn so it survives respawn.
	if (IsLocalPlayerController() && !HudWidget && *HudWidgetClass)
	{
		HudWidget = CreateWidget<UPlayerHudWidget>(this, HudWidgetClass);
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

void AMainPlayerController::SetupInputComponent()
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

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		InPawn->OnDestroyed.AddDynamic(this, &AMainPlayerController::OnPawnDestroyed);
		// Update respawn to last possessed location if still identity (first spawn / checkpoint-less run).
		if (RespawnTransform.Equals(FTransform::Identity) || RespawnTransform.GetLocation().IsNearlyZero())
		{
			RespawnTransform = InPawn->GetActorTransform();
			RespawnTransform.SetScale3D(FVector::OneVector);
		}
	}
}

void AMainPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	RespawnTransform = NewRespawn;
}

bool AMainPlayerController::SaveProgress()
{
	// NOTE: locals intentionally NOT named Pawn/Character: AController owns
	// members with those names and C4458 is an error in this build.
	APawn* TargetPawn = GetPawn();
	const APlayerCharacter* TargetCharacter = Cast<APlayerCharacter>(TargetPawn);
	if (!TargetPawn || !TargetCharacter)
	{
		return false;
	}

	UPlayerSaveGame* Save = Cast<UPlayerSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPlayerSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	Save->SlotName = SaveSlotName;
	const FTransform PawnTransform = TargetPawn->GetActorTransform();
	Save->PlayerLocation = PawnTransform.GetLocation();
	Save->PlayerRotation = PawnTransform.Rotator();
	Save->PlayerHealth = TargetCharacter->GetHealth();
	if (const UInventoryComponent* Inv = TargetCharacter->GetInventory())
	{
		Save->Inventory = Inv->Items;
	}
	if (const UProgressionComponent* Prog = TargetCharacter->GetProgression())
	{
		Save->Level = Prog->Level;
		Save->Experience = Prog->Experience;
	}

	return UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
}

bool AMainPlayerController::LoadProgress()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return false;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
	UPlayerSaveGame* Save = Cast<UPlayerSaveGame>(Loaded);
	APawn* TargetPawn = GetPawn();
	APlayerCharacter* TargetCharacter = Cast<APlayerCharacter>(TargetPawn);
	if (!Save || !TargetPawn || !TargetCharacter)
	{
		return false;
	}

	const FTransform SavedTransform(Save->PlayerRotation, Save->PlayerLocation, FVector::OneVector);
	TargetPawn->SetActorLocationAndRotation(Save->PlayerLocation, Save->PlayerRotation, false, nullptr, ETeleportType::TeleportPhysics);
	TargetCharacter->SetHealth(Save->PlayerHealth);
	if (UInventoryComponent* Inv = TargetCharacter->GetInventory())
	{
		Inv->SetItems(Save->Inventory);
	}
	if (UProgressionComponent* Prog = TargetCharacter->GetProgression())
	{
		Prog->SetProgress(Save->Level, Save->Experience);
	}
	// Loading doubles as a checkpoint: dying after a load returns here.
	RespawnTransform = SavedTransform;
	RespawnTransform.SetScale3D(FVector::OneVector);
	return true;
}

void AMainPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	TSubclassOf<APlayerCharacter> SpawnClass = CharacterClass;

	if (!SpawnClass)
	{
		if (const AMainGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMainGameMode>() : nullptr)
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
		if (APlayerCharacter* Respawned = World->SpawnActor<APlayerCharacter>(SpawnClass, RespawnTransform))
		{
			Possess(Respawned);
		}
	}
}
