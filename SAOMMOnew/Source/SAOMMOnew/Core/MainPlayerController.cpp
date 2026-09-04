// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
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

	// Fallback actions: same FObjectFinder defaults as the character, so the
	// transient mapping below always matches the pawn's bindings.
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveObj(TEXT("/Game/Input/IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookObj(TEXT("/Game/Input/IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpObj(TEXT("/Game/Input/IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction> AttackObj(TEXT("/Game/Input/IA_Attack"));
	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleCameraObj(TEXT("/Game/Input/IA_ToggleCamera"));
	static ConstructorHelpers::FObjectFinder<UInputAction> InteractObj(TEXT("/Game/Input/IA_Interact"));
	if (MoveObj.Succeeded()) { MoveAction = MoveObj.Object; }
	if (LookObj.Succeeded()) { LookAction = LookObj.Object; }
	if (JumpObj.Succeeded()) { JumpAction = JumpObj.Object; }
	if (AttackObj.Succeeded()) { AttackAction = AttackObj.Object; }
	if (ToggleCameraObj.Succeeded()) { ToggleCameraAction = ToggleCameraObj.Object; }
	if (InteractObj.Succeeded()) { InteractAction = InteractObj.Object; }
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
				int32 Added = 0;
				for (UInputMappingContext* Context : DefaultMappingContexts)
				{
					if (Context)
					{
						Subsystem->AddMappingContext(Context, 0);
						++Added;
					}
				}
				// No content mapping assigned (e.g. broken/unmigrated IMC
				// asset): fall back to the transient C++ mapping so the game
				// stays playable with zero Editor setup.
				if (Added == 0)
				{
					BuildFallbackMapping();
					if (FallbackMapping)
					{
						Subsystem->AddMappingContext(FallbackMapping, 0);
					}
				}
			}
		}
	}
}

void AMainPlayerController::BuildFallbackMapping()
{
	if (FallbackMapping)
	{
		return;
	}

	FallbackMapping = NewObject<UInputMappingContext>(this);
	if (!FallbackMapping)
	{
		return;
	}

	// NOTE: never hold the FEnhancedActionKeyMapping& returned by MapKey
	// across further MapKey calls (array realloc would dangle it — this
	// corrupted mappings before). Record indices, configure afterwards via
	// GetMapping(Index), whose indices stay stable.
	auto AddKey = [&](UInputAction* Action, const FKey& Key) -> int32
	{
		if (!Action)
		{
			return INDEX_NONE;
		}
		const int32 Index = FallbackMapping->GetMappings().Num();
		FallbackMapping->MapKey(Action, Key);
		return Index;
	};

	const int32 W = AddKey(MoveAction, EKeys::W);
	const int32 D = AddKey(MoveAction, EKeys::D);
	const int32 S = AddKey(MoveAction, EKeys::S);
	const int32 A = AddKey(MoveAction, EKeys::A);
	// Mouse as ONE Vector2D key (stock template pattern): separate MouseX /
	// MouseY axis keys inject on the wrong axis. Negate Y for standard look.
	const int32 Mouse = AddKey(LookAction, EKeys::Mouse2D);
	AddKey(JumpAction, EKeys::SpaceBar);
	AddKey(AttackAction, EKeys::LeftMouseButton);
	AddKey(ToggleCameraAction, EKeys::V);
	AddKey(InteractAction, EKeys::E);
	// I opens the inventory directly: same action, and OnInteract toggles
	// the screen whenever nothing is in reach anyway.
	AddKey(InteractAction, EKeys::I);

	auto Negate = [&](int32 Index, bool bX, bool bY)
	{
		if (Index == INDEX_NONE)
		{
			return;
		}
		UInputModifierNegate* Neg = NewObject<UInputModifierNegate>(FallbackMapping);
		Neg->bX = bX;
		Neg->bY = bY;
		Neg->bZ = false;
		FallbackMapping->GetMapping(Index).Modifiers.Add(Neg);
	};

	// Digital keys inject (1,0) into 2D actions, so W/S must first swizzle
	// the press onto Y (without this both go right). S additionally negates.
	auto SwizzleY = [&](int32 Index)
	{
		if (Index == INDEX_NONE)
		{
			return;
		}
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(FallbackMapping);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		FallbackMapping->GetMapping(Index).Modifiers.Add(Swizzle);
	};

	// S = backward (-Y), A = left (-X), mouse pitch negated for standard look.
	SwizzleY(W);
	SwizzleY(S);
	Negate(S, false, true);
	Negate(A, true, false);
	Negate(Mouse, false, true);
	(void)D;
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

void AMainPlayerController::ToggleInventory()
{
	if (HudWidget)
	{
		HudWidget->ToggleInventory();
	}
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
