// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRCharacter.h"
#include "SAOMMOnew.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "MotionControllerComponent.h"
#include "Sword.h"
#include "CombatComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->bUsePawnControlRotation = false;

	// Tracked hand anchors (Band 2 §7, audit P6): motion controllers bound to
	// the standard OpenXR grip poses. Without tracking they keep these offsets
	// (the component falls back to its current relative transform).
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(VROrigin);
	LeftHand->SetTrackingMotionSource(FName(TEXT("Left")));
	LeftHand->SetRelativeLocation(FVector(0.0f, -30.0f, 0.0f));

	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(VROrigin);
	RightHand->SetTrackingMotionSource(FName(TEXT("Right")));
	RightHand->SetRelativeLocation(FVector(0.0f, 30.0f, 0.0f));

	InputFrame = CreateDefaultSubobject<UInputFrameComponent>(TEXT("InputFrame"));

	// Combat (Band 2 §8): arms the sword from swing velocity — the gate only
	// counts velocity on motion devices, so this is the designed VR feed
	// (audit P8; the component itself is not modified).
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// Desktop-fallback actions (backlog #16): same FObjectFinder defaults as
	// APlayerCharacter and AMainPlayerController, so the pawn's bindings and
	// the controller's fallback mapping always agree on the assets.
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveObj(TEXT("/Game/Input/IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookObj(TEXT("/Game/Input/IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpObj(TEXT("/Game/Input/IA_Jump"));
	static ConstructorHelpers::FObjectFinder<UInputAction> AttackObj(TEXT("/Game/Input/IA_Attack"));
	if (MoveObj.Succeeded()) { MoveAction = MoveObj.Object; }
	if (LookObj.Succeeded()) { LookAction = LookObj.Object; }
	if (JumpObj.Succeeded()) { JumpAction = JumpObj.Object; }
	if (AttackObj.Succeeded()) { AttackAction = AttackObj.Object; }

	DefaultSwordClass = ASword::StaticClass();
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (InputFrame)
	{
		InputFrame->CurrentFrame.SourceDevice = EInputDevice::VRController;
	}

	// Band 3 §20 P7 ("Hold a sword"), audit G4: spawn the sword and bind it to
	// the tracked right hand — in VR the motion controller IS the hand, so no
	// skeleton socket exists. Grip alignment against the controller axes still
	// needs a headset pass (proof item, backlog #8).
	if (DefaultSwordClass && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EquippedSword = GetWorld()->SpawnActor<ASword>(DefaultSwordClass, GetActorTransform(), Params);
		if (EquippedSword)
		{
			EquippedSword->AttachToComponent(
				RightHand ? static_cast<USceneComponent*>(RightHand) : GetRootComponent(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			// The VR pawn renders no body: a floating sword shadow would read as
			// a bug (same reasoning as the desktop first-person path, Sword.h).
			EquippedSword->SetShadowCasting(false);
			EquippedSword->SetOwnerActor(this);
			if (CombatComponent)
			{
				CombatComponent->SetSword(EquippedSword);
			}
		}
	}

	// With a full OpenXR build the camera is driven by the HMD and the hands by
	// motion controllers; Tick mirrors those transforms into FInputFrame so
	// gameplay stays device-independent (Band 2 §4, §7).
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTarget(this);
	}
}

void AVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Mirror the tracked HMD/hand poses into the device-independent frame
	// (Band 2 §3/§4): gameplay reads FInputFrame, never OpenXR types. Poses are
	// world space; on non-VR devices FInputFrame::Reset() keeps them neutral.
	if (InputFrame && LeftHand && RightHand && VRCamera)
	{
		FInputFrame& Frame = InputFrame->CurrentFrame;
		Frame.HeadRotation = VRCamera->GetComponentRotation();
		Frame.LeftHandPosition = LeftHand->GetComponentLocation();
		Frame.LeftHandRotation = LeftHand->GetComponentRotation();
		Frame.RightHandPosition = RightHand->GetComponentLocation();
		Frame.RightHandRotation = RightHand->GetComponentRotation();
	}
}

bool AVRCharacter::IsXRSessionActive()
{
	// Single XR activation gate (ADR-017c): GEngine->XRSystem (Engine.h) is the
	// registered IXRTrackingSystem — MainGameMode's initial pawn choice and
	// MainPlayerController's respawn both ask this one question.
	return GEngine && GEngine->XRSystem.IsValid();
}

float AVRCharacter::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	// Band 3 §11 — same clamp semantics as APlayerCharacter::TakeDamage.
	if (CurrentHealth <= 0.0f || Damage <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AVRCharacter::Die()
{
	// Band 3 §11 (Health = 0 → Death): drop the blade as physical debris like
	// the desktop pawn, then destroy — AMainPlayerController::OnPawnDestroyed
	// (pawn-type-agnostic) shows the death screen and drives DoRespawn, whose
	// XR branch respawns this pawn (ADR-017c).
	if (EquippedSword)
	{
		EquippedSword->DropPhysics();
		EquippedSword = nullptr;
	}
	Destroy();
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVRCharacter::OnMove);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVRCharacter::OnLook);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AVRCharacter::OnAttack);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AVRCharacter::OnJump);
		}

		// Sprint: transient action + Shift mapping built in code, so the
		// feature ships with zero InputAction/IMC assets (same guarantee as
		// the code-only HUD and the desktop pawn).
		EnsureSprintMapping();
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AVRCharacter::OnSprintStarted);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVRCharacter::OnSprintStopped);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AVRCharacter::OnSprintStopped);
		}
	}

	// Smoke marker (backlog #16): proves the fallback ran on the possessed
	// pawn — grep "VR input" in a -game boot log next to pawn=VRCharacter.
	UE_LOG(LogGame, Display, TEXT("VR input: desktop fallback bound (move=%d look=%d jump=%d attack=%d sprint=%d)"),
		MoveAction ? 1 : 0, LookAction ? 1 : 0, JumpAction ? 1 : 0,
		AttackAction ? 1 : 0, SprintAction ? 1 : 0);
}

void AVRCharacter::EnsureSprintMapping()
{
	if (SprintAction && SprintMapping)
	{
		// Already built; still make sure the local player has the mapping
		// (respawn creates a fresh pawn against the same local player).
	}
	else
	{
		SprintAction = NewObject<UInputAction>(this, TEXT("IA_Sprint_Runtime"));
		SprintMapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Sprint_Runtime"));
		if (SprintAction && SprintMapping)
		{
			SprintAction->ValueType = EInputActionValueType::Boolean;
			SprintMapping->MapKey(SprintAction, EKeys::LeftShift);
		}
		else
		{
			SprintAction = nullptr;
			SprintMapping = nullptr;
			return;
		}
	}

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (!Sub->HasMappingContext(SprintMapping))
				{
					Sub->AddMappingContext(SprintMapping, 0);
				}
			}
		}
	}
}

void AVRCharacter::OnMove(const FInputActionValue& Value)
{
	// Same contract as APlayerCharacter::OnMove: move along the control
	// rotation and mirror the vector into the device-independent frame.
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (GetController() && InputFrame)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		InputFrame->CurrentFrame.Move = MovementVector;
	}
}

void AVRCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() && InputFrame)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		// The VR camera ignores control rotation by design (Band 2 §7: the HMD
		// drives it), so mouse look would otherwise be invisible here. Drive the
		// pawn yaw from the control rotation (keeps the view and the WASD
		// direction in sync) and pitch the camera directly. Input-driven only:
		// while an HMD streams poses the tracking system owns the camera
		// transform every frame, so headset behaviour is unchanged.
		FRotator ActorRotation = GetActorRotation();
		ActorRotation.Yaw = GetController()->GetControlRotation().Yaw;
		SetActorRotation(ActorRotation);

		FRotator CameraRotation = VRCamera->GetRelativeRotation();
		CameraRotation.Pitch = FMath::Clamp(GetController()->GetControlRotation().Pitch, -89.0f, 89.0f);
		VRCamera->SetRelativeRotation(CameraRotation);

		InputFrame->CurrentFrame.Turn = LookAxisVector;
	}
}

void AVRCharacter::OnAttack(const FInputActionValue& Value)
{
	// LMB arms the same rising edge the desktop pawn uses; the device-
	// independent CombatComponent consumes it (Band 2 §8, audit P8 untouched).
	if (InputFrame)
	{
		InputFrame->CurrentFrame.bAttack = true;
	}
}

void AVRCharacter::OnJump(const FInputActionValue& Value)
{
	Jump();
}

void AVRCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = SprintSpeed;
	}
}

void AVRCharacter::OnSprintStopped(const FInputActionValue& Value)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeed;
	}
}
