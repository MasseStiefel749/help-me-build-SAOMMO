// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "InputFrameComponent.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UCombatComponent;
class UInventoryComponent;
class UProgressionComponent;
class UInteractionComponent;
class ASword;
struct FInputActionValue;

/** Broadcast when the player character dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerCharacterDied);

/**
 *  Base SAOMMO player character (Band 2 §5).
 *
 *  Designed to operate in both desktop and VR modes without being recreated.
 *  Desktop presentation supports First Person and Third Person camera modes
 *  that can be switched at runtime without recreating the character (Band 2 §6).
 *
 *  Input is gathered into a device-independent FInputFrame via
 *  UInputFrameComponent rather than being consumed directly from a
 *  hardware-specific input system (Band 2 §4).
 */
UCLASS(Blueprintable)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character (Third Person). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Device-independent input frame producer (Band 2 §4). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInputFrameComponent* InputFrame;

	/** Combat component; owns swing detection and arms the sword (Band 2 §8). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	/** Inventory: owns the Fight->Loot collection (Band 3 §13). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	/** Progression: records XP toward Improve (Band 3 §12). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProgressionComponent* ProgressionComponent;

	/** Interaction: forward trace for pick up / activate (Band 3 §6). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInteractionComponent* InteractionComponent;

	/** Sword class spawned and attached on BeginPlay. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ASword> DefaultSwordClass;

	/** Currently equipped sword (spawned at runtime). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASword> EquippedSword;

	/** Maximum health on spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = 1, ClampMax = 1000))
	float MaxHealth = 10.0f;

	/** Current health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;

	/** Broadcast when the character dies. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerCharacterDied OnDied;

protected:

	/** Current desktop camera mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	ECameraMode CameraMode = ECameraMode::ThirdPerson;

	/** Move Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Camera mode toggle Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleCameraAction;

	/** Attack Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/** Interact (pick up / activate) Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	/** Jump Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Eye height used when the camera is in First Person mode. */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = 0, ClampMax = 300, Units = "cm"))
	float FirstPersonEyeHeight = 70.0f;

	/** Boom length used in Third Person mode. */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float ThirdPersonBoomLength = 300.0f;

public:

	APlayerCharacter();

	/** Returns the device-independent input frame component. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputFrameComponent* GetInputFrame() const { return InputFrame; }

	/** Returns the inventory component (Loot). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryComponent* GetInventory() const { return InventoryComponent; }
	/** Returns the progression component (Improve). */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	UProgressionComponent* GetProgression() const { return ProgressionComponent; }

	/** Returns the interaction component. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UInteractionComponent* GetInteraction() const { return InteractionComponent; }

	/** Returns current health. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetHealth() const { return CurrentHealth; }

	/** Returns maximum health. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetMaxHealth() const { return MaxHealth; }

	/** Restores health up to MaxHealth. Returns the amount actually healed. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float Heal(float Amount);

	/** Directly sets health clamped to [0, MaxHealth] (save/load). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetHealth(float NewHealth) { CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth); }

	/** Returns the current camera mode. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	ECameraMode GetCameraMode() const { return CameraMode; }

	/** Switches the desktop camera mode without recreating the character. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewMode);

protected:

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnToggleCamera(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);

	/** Repositions the camera for the active mode. */
	void UpdateCameraAttachment();

	virtual float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	/** Handles death and removal from the level. */
	void Die();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
