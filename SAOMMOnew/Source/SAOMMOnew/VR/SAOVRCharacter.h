// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAOMMOCoreTypes.h"
#include "SAOMMOInputFrameComponent.h"
#include "SAOVRCharacter.generated.h"

class USceneComponent;
class UCameraComponent;

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
 *  Hand/controller transforms are exposed as SceneComponents so the same
 *  FSAOInputFrame (LeftHandPosition / LeftHandRotation / ...) can be fed by real
 *  OpenXR motion controllers later without changing the gameplay layer. The
 *  character shares the device-independent input frame with the desktop
 *  character (Band 2 §4).
 */
UCLASS(abstract, Blueprintable)
class ASAOVRCharacter : public ACharacter
{
	GENERATED_BODY()

	/** VR root; recentered so the player's real-world position maps to the level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* VROrigin;

	/** Head / HMD camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* VRCamera;

	/** Left hand / controller anchor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* LeftHand;

	/** Right hand / controller anchor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* RightHand;

	/** Device-independent input frame producer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USAOMMOInputFrameComponent* InputFrame;

public:

	ASAOVRCharacter();

	UFUNCTION(BlueprintCallable, Category = "VR")
	USceneComponent* GetLeftHand() const { return LeftHand; }

	UFUNCTION(BlueprintCallable, Category = "VR")
	USceneComponent* GetRightHand() const { return RightHand; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	USAOMMOInputFrameComponent* GetInputFrame() const { return InputFrame; }

protected:

	virtual void BeginPlay() override;
};
