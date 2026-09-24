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

public:

	AVRCharacter();

	UFUNCTION(BlueprintCallable, Category = "VR")
	UMotionControllerComponent* GetLeftHand() const { return LeftHand; }

	UFUNCTION(BlueprintCallable, Category = "VR")
	UMotionControllerComponent* GetRightHand() const { return RightHand; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	UInputFrameComponent* GetInputFrame() const { return InputFrame; }

protected:

	virtual void BeginPlay() override;

	/** Mirrors tracked HMD/hand poses into the shared FInputFrame (Band 2 §4). */
	virtual void Tick(float DeltaSeconds) override;
};
