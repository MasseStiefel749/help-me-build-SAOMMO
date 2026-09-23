// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SharedTypes.generated.h"

/**
 *  Device that produced the current input frame.
 *  Keeps the gameplay layer independent of the physical hardware (Band 2 §4).
 */
UENUM(BlueprintType)
enum class EInputDevice : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	KeyboardMouse	UMETA(DisplayName = "Keyboard / Mouse"),
	VRController	UMETA(DisplayName = "VR Controller"),
	BudgetVR		UMETA(DisplayName = "Budget VR")
};

/**
 *  Desktop camera presentation mode (Band 2 §6).
 *  Switching must not recreate the player character.
 */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	FirstPerson		UMETA(DisplayName = "First Person"),
	ThirdPerson		UMETA(DisplayName = "Third Person")
};

/**
 *  Device-independent snapshot of gameplay-relevant input.
 *
 *  Per Band 2 §3 the gameplay layer should depend on this frame rather than on
 *  a specific hardware platform. Any input provider (keyboard/mouse, VR
 *  controllers, Budget VR bridge) fills the same structure before gameplay
 *  consumes it (Band 2 §4).
 */
USTRUCT(BlueprintType)
struct FInputFrame
{
	GENERATED_BODY()

	/** Planar locomotion intent, already in character space (X = forward, Y = right). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D Move = FVector2D::ZeroVector;

	/** Look / turn intent (X = yaw, Y = pitch). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector2D Turn = FVector2D::ZeroVector;

	/** Head / HMD orientation. Identity on non-VR devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FRotator HeadRotation = FRotator::ZeroRotator;

	/** Tracked left-hand orientation (VR). Identity on non-VR devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FRotator LeftHandRotation = FRotator::ZeroRotator;

	/** Tracked right-hand orientation (VR). Identity on non-VR devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FRotator RightHandRotation = FRotator::ZeroRotator;

	/** Tracked left-hand position (VR). Zero on non-VR devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector LeftHandPosition = FVector::ZeroVector;

	/** Tracked right-hand position (VR). Zero on non-VR devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FVector RightHandPosition = FVector::ZeroVector;

	/** Attack / swing intent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bAttack = false;

	/** Menu / system intent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bMenu = false;

	/** Source device that produced this frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EInputDevice SourceDevice = EInputDevice::Unknown;

	/** Resets all fields to their neutral state. */
	void Reset()
	{
		Move = FVector2D::ZeroVector;
		Turn = FVector2D::ZeroVector;
		HeadRotation = FRotator::ZeroRotator;
		LeftHandRotation = FRotator::ZeroRotator;
		RightHandRotation = FRotator::ZeroRotator;
		LeftHandPosition = FVector::ZeroVector;
		RightHandPosition = FVector::ZeroVector;
		bAttack = false;
		bMenu = false;
		SourceDevice = EInputDevice::Unknown;
	}
};
