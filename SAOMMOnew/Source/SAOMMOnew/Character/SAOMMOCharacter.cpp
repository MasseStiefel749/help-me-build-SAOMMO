// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOMMOCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SAOSword.h"
#include "SAOMMOCombatComponent.h"
#include "SAOMMOInventoryComponent.h"
#include "SAOMMOProgressionComponent.h"
#include "SAOMMOInteractionComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASAOMMOCharacter::ASAOMMOCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = ThirdPersonBoomLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->bUsePawnControlRotation = false;

	InputFrame = CreateDefaultSubobject<USAOMMOInputFrameComponent>(TEXT("InputFrame"));

	CombatComponent = CreateDefaultSubobject<USAOMMOCombatComponent>(TEXT("CombatComponent"));

	InventoryComponent = CreateDefaultSubobject<USAOMMOInventoryComponent>(TEXT("InventoryComponent"));

	ProgressionComponent = CreateDefaultSubobject<USAOMMOProgressionComponent>(TEXT("ProgressionComponent"));

	InteractionComponent = CreateDefaultSubobject<USAOMMOInteractionComponent>(TEXT("InteractionComponent"));

	DefaultSwordClass = ASAOSword::StaticClass();
}

void ASAOMMOCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	UpdateCameraAttachment();

	if (InputFrame)
	{
		InputFrame->CurrentFrame.SourceDevice = ESAOInputDevice::KeyboardMouse;
	}

	if (DefaultSwordClass && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EquippedSword = GetWorld()->SpawnActor<ASAOSword>(DefaultSwordClass, GetActorTransform(), Params);
		if (EquippedSword)
		{
			// Prefer hand socket if skeletal mesh has it; fall back to root for blockout.
			if (USkeletalMeshComponent* Skel = GetMesh())
			{
				if (Skel->DoesSocketExist(TEXT("hand_rSocket")) || Skel->DoesSocketExist(TEXT("hand_r")))
				{
					const FName Socket = Skel->DoesSocketExist(TEXT("hand_rSocket")) ? TEXT("hand_rSocket") : TEXT("hand_r");
					EquippedSword->AttachToComponent(Skel, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
				}
				else
				{
					EquippedSword->AttachToComponent(Skel, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
			}
			else
			{
				EquippedSword->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			EquippedSword->SetOwnerActor(this);
			if (CombatComponent)
			{
				CombatComponent->SetSword(EquippedSword);
			}
		}
	}
}

void ASAOMMOCharacter::SetCameraMode(ESAOCameraMode NewMode)
{
	if (CameraMode == NewMode)
	{
		return;
	}

	CameraMode = NewMode;
	UpdateCameraAttachment();
}

void ASAOMMOCharacter::UpdateCameraAttachment()
{
	if (!FollowCamera || !CameraBoom)
	{
		return;
	}

	if (CameraMode == ESAOCameraMode::FirstPerson)
	{
		// Runtime attach: SetupAttachment only valid in construction; use AttachToComponent.
		FollowCamera->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, FirstPersonEyeHeight));
		FollowCamera->bUsePawnControlRotation = true;
		CameraBoom->bDoCollisionTest = false;
	}
	else
	{
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
		FollowCamera->SetRelativeLocation(FVector::ZeroVector);
		FollowCamera->bUsePawnControlRotation = false;
		CameraBoom->TargetArmLength = ThirdPersonBoomLength;
		CameraBoom->bDoCollisionTest = true;
	}
}

void ASAOMMOCharacter::OnMove(const FInputActionValue& Value)
{
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

void ASAOMMOCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() && InputFrame)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		InputFrame->CurrentFrame.Turn = LookAxisVector;
	}
}

void ASAOMMOCharacter::OnToggleCamera(const FInputActionValue& Value)
{
	SetCameraMode(CameraMode == ESAOCameraMode::FirstPerson
		? ESAOCameraMode::ThirdPerson
		: ESAOCameraMode::FirstPerson);
}

void ASAOMMOCharacter::OnAttack(const FInputActionValue& Value)
{
	if (InputFrame)
	{
		InputFrame->CurrentFrame.bAttack = true;
	}
}

void ASAOMMOCharacter::OnInteract(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void ASAOMMOCharacter::OnJump(const FInputActionValue& Value)
{
	Jump();
}

float ASAOMMOCharacter::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
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

float ASAOMMOCharacter::Heal(float Amount)
{
	if (Amount <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float Missing = MaxHealth - CurrentHealth;
	const float Applied = FMath::Min(Missing, Amount);
	CurrentHealth += Applied;
	return Applied;
}

void ASAOMMOCharacter::Die()
{
	OnDied.Broadcast();
	// Respawn path (PlayerController) creates a fresh pawn; destroy the
	// equipped sword with the old body so no orphan weapon actors linger.
	if (EquippedSword)
	{
		EquippedSword->Destroy();
		EquippedSword = nullptr;
	}
	Destroy();
}

void ASAOMMOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASAOMMOCharacter::OnMove);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASAOMMOCharacter::OnLook);
		}
		if (ToggleCameraAction)
		{
			EnhancedInput->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ASAOMMOCharacter::OnToggleCamera);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &ASAOMMOCharacter::OnAttack);
		}
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASAOMMOCharacter::OnInteract);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASAOMMOCharacter::OnJump);
		}
	}
}
