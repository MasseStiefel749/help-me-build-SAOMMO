// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UObject/ConstructorHelpers.h"
#include "Sword.h"
#include "CombatComponent.h"
#include "InventoryComponent.h"
#include "ProgressionComponent.h"
#include "InteractionComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
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

	InputFrame = CreateDefaultSubobject<UInputFrameComponent>(TEXT("InputFrame"));

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	ProgressionComponent = CreateDefaultSubobject<UProgressionComponent>(TEXT("ProgressionComponent"));

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	DefaultSwordClass = ASword::StaticClass();

	// Fallback input actions so the pawn is playable with zero Blueprint
	// setup. All six assets exist and load (verified headless); a Blueprint
	// child may still override any of them.
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

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	UpdateCameraAttachment();

	if (InputFrame)
	{
		InputFrame->CurrentFrame.SourceDevice = EInputDevice::KeyboardMouse;
	}

	if (DefaultSwordClass && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EquippedSword = GetWorld()->SpawnActor<ASword>(DefaultSwordClass, GetActorTransform(), Params);
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

void APlayerCharacter::SetCameraMode(ECameraMode NewMode)
{
	if (CameraMode == NewMode)
	{
		return;
	}

	CameraMode = NewMode;
	UpdateCameraAttachment();
}

void APlayerCharacter::UpdateCameraAttachment()
{
	if (!FollowCamera || !CameraBoom)
	{
		return;
	}

	if (CameraMode == ECameraMode::FirstPerson)
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

void APlayerCharacter::OnMove(const FInputActionValue& Value)
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

void APlayerCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController() && InputFrame)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		InputFrame->CurrentFrame.Turn = LookAxisVector;
	}
}

void APlayerCharacter::OnToggleCamera(const FInputActionValue& Value)
{
	SetCameraMode(CameraMode == ECameraMode::FirstPerson
		? ECameraMode::ThirdPerson
		: ECameraMode::FirstPerson);
}

void APlayerCharacter::OnAttack(const FInputActionValue& Value)
{
	if (InputFrame)
	{
		InputFrame->CurrentFrame.bAttack = true;
	}
}

void APlayerCharacter::OnInteract(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void APlayerCharacter::OnJump(const FInputActionValue& Value)
{
	Jump();
}

float APlayerCharacter::TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
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

float APlayerCharacter::Heal(float Amount)
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

void APlayerCharacter::Die()
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

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnMove);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnLook);
		}
		if (ToggleCameraAction)
		{
			EnhancedInput->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnToggleCamera);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::OnAttack);
		}
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::OnInteract);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::OnJump);
		}
	}
}
