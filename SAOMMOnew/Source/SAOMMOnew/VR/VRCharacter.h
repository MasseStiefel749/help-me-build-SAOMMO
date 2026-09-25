// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "InputFrameComponent.h"
#include "VRCharacter.generated.h"

class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UCombatComponent;
class ASword;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 *  VR player character (Band 2 §7).
 *
 *  Establishes the documented hierarchy:
 *    SAOMMO Character
 *      └── VROrigin
 *            ├── Camera
 *            ├── Left Hand
 *            └── Right Hand
 *
 *  Hand/controller anchors are UMotionControllerComponents (MotionSource
 *  "Left"/"Right") so they track real OpenXR controllers; their poses are
 *  mirrored into the shared FInputFrame (LeftHandPosition / LeftHandRotation /
 *  ...) every tick, keeping the gameplay layer device-independent (Band 2 §3,
 *  §4). The character shares that input frame with the desktop character.
 *
 *  Block 3c (Band 3 §20 P7/P8, audit G4): spawns a sword attached to the right
 *  tracked hand and owns a CombatComponent that arms it from swing velocity.
 *  The component itself is device-independent by design and stays untouched
 *  (audit P8: "do not touch CombatComponent for Block 3").
 *
 *  Backlog #16 (desktop input fallback): binds the same enhanced-input
 *  actions as APlayerCharacter (Move/Look/Jump/Attack + code-only Sprint),
 *  so a desktop session stays controllable and recordable while an XR system
 *  registers this pawn (Nachttest Fund 8: pawn spawned immobile whenever the
 *  Oculus runtime ran, even with no headset worn).
 */
UCLASS(Blueprintable)
class AVRCharacter : public ACharacter
{
	GENERATED_BODY()

	/** VR root; recentered so the player's real-world position maps to the level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* VROrigin;

	/** Head / HMD camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* VRCamera;

	/** Tracked left hand / controller (MotionSource = "Left", Band 2 §7). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* LeftHand;

	/** Tracked right hand / controller (MotionSource = "Right"). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* RightHand;

	/** Device-independent input frame producer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInputFrameComponent* InputFrame;

	/** Combat component; owns swing detection and arms the sword (Band 2 §8). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	/** Sword class spawned and attached to the right hand on BeginPlay. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ASword> DefaultSwordClass;

	/** Currently equipped sword (spawned at runtime). */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ASword> EquippedSword;

	/** Maximum health on spawn (Band 3 §11; mirrors the desktop contract). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true", ClampMin = 1, ClampMax = 1000))
	float MaxHealth = 10.0f;

	/** Current health; set to MaxHealth on BeginPlay, reaches 0 on death. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;

public:

	AVRCharacter();

	/** The single XR activation gate (ADR-017c): true while an XR system (HMD)
	 *  is registered with the engine. Shared by MainGameMode (initial spawn) and
	 *  MainPlayerController (respawn) so the two paths cannot drift. */
	static bool IsXRSessionActive();

	UFUNCTION(BlueprintCallable, Category = "VR")
	UMotionControllerComponent* GetLeftHand() const { return LeftHand; }

	UFUNCTION(BlueprintCallable, Category = "VR")
	UMotionControllerComponent* GetRightHand() const { return RightHand; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputFrameComponent* GetInputFrame() const { return InputFrame; }

	/** Equipped sword (nullptr until BeginPlay). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	ASword* GetEquippedSword() const { return EquippedSword; }

	/** Band 3 §11 damage entry point (Enemy::PerformAttack calls TakeDamage). */
	virtual float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	/** Health = 0 → drop the sword as debris → destroy the pawn (Band 3 §11). */
	void Die();

protected:

	virtual void BeginPlay() override;

	/** Mirrors tracked HMD/hand poses into the shared FInputFrame (Band 2 §4). */
	virtual void Tick(float DeltaSeconds) override;

	/** Desktop-fallback wiring (backlog #16): mirrors APlayerCharacter's
	 *  enhanced-input setup — same assets, same trigger events. */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Move Input Action (same asset as the desktop pawn). */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action (mouse). */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Attack Input Action (LMB → FInputFrame::bAttack → CombatComponent). */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/** Jump Input Action. */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Sprint action + mapping built at possession time (no InputAction asset needed). */
	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> SprintMapping = nullptr;

	/** Walk speed; applied on construction (mirrors the desktop contract — the
	 *  pawn previously kept the engine default because nothing could drive it). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = 100, ClampMax = 1200, Units = "cm/s"))
	float WalkSpeed = 400.0f;

	/** Sprint speed while the sprint key is held (Shift, mapped in code). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = 100, ClampMax = 2000, Units = "cm/s"))
	float SprintSpeed = 650.0f;

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintStopped(const FInputActionValue& Value);

	/** Builds the transient Shift sprint mapping (once per pawn) and registers it. */
	void EnsureSprintMapping();
};
