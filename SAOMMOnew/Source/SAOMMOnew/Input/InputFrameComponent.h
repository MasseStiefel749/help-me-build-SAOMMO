// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "InputFrameComponent.generated.h"

/** Broadcast every tick with the latest device-independent input frame. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputFrameUpdated, const FInputFrame&, Frame);

/**
 *  Collects physical input into a device-independent FInputFrame and
 *  broadcasts it to the gameplay layer (Band 2 §4).
 *
 *  Any input provider (keyboard/mouse, VR controllers, Budget VR bridge) may
 *  write into CurrentFrame. Gameplay subscribes to OnFrameUpdated instead of
 *  binding directly to a hardware-specific input system.
 */
UCLASS(ClassGroup = (Game), meta = (BlueprintSpawnableComponent))
class UInputFrameComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInputFrameComponent();

	/** Latest device-independent input frame. Written by input providers, read by gameplay. */
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FInputFrame CurrentFrame;

	/** Broadcast each tick with the current frame. */
	UPROPERTY(BlueprintAssignable, Category = "Input")
	FOnInputFrameUpdated OnFrameUpdated;

	/** Convenience: clear the frame (e.g. on possession loss). */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ClearFrame() { CurrentFrame.Reset(); }

protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
