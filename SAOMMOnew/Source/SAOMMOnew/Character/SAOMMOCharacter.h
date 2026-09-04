// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAOMMOCoreTypes.h"
#include "SAOMMOInputFrameComponent.h"
#include "SAOMMOCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class USAOMMOCombatComponent;
class ASAOSword;
struct FInputActionValue;

/** Broadcast when the player character dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSAOMMOCharacterDied);

/**
 *  Base SAOMMO player character (Band 2 §5).
 *
 *  Designed to operate in both desktop and VR modes without being recreated.
 *  Desktop presentation supports First Person and Third Person camera modes
 *  that can be switched at runtime without recreating the character (Band 2 §6).
 *
 *  Input is gathered into a device-independent FSAOInputFrame via
 *  USAOMMOInputFrameComponent rather than being consumed directly from a
 *  hardware-specific input system (Band 2 §4).
 */
UCLASS(Blueprintable)
class ASAOMMOCharacter : public ACharacter
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
	USAOMMOInputFrameComponent* InputFrame;

	/** Combat component; owns swing detection and arms the sword (Band 2 §8). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USAOMMOCombatComponent* CombatComponent;

	/** Sword class spawned and attached on BeginPlay. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ASAOSword> DefaultSwordClass;

	/** Currently equipped sword (spawned at runtime). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASAOSword> EquippedSword;

	/** Maximum health on spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = 1, ClampMax = 1000))
	float MaxHealth = 10.0f;

	/** Current health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;

	/** Broadcast when the character dies. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSAOMMOCharacterDied OnDied;

protected:

	/** Current desktop camera mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	ESAOCameraMode CameraMode = ESAOCameraMode::ThirdPerson;

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

	ASAOMMOCharacter();

	/** Returns the device-independent input frame component. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	USAOMMOInputFrameComponent* GetInputFrame() const { return InputFrame; }

	/** Returns the current camera mode. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	ESAOCameraMode GetCameraMode() const { return CameraMode; }

	/** Switches the desktop camera mode without recreating the character. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ESAOCameraMode NewMode);

protected:

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnToggleCamera(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
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
