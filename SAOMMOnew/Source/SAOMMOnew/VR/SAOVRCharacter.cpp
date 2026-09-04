// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOVRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

ASAOVRCharacter::ASAOVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->bUsePawnControlRotation = false;

	LeftHand = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(VROrigin);
	LeftHand->SetRelativeLocation(FVector(0.0f, -30.0f, 0.0f));

	RightHand = CreateDefaultSubobject<USceneComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(VROrigin);
	RightHand->SetRelativeLocation(FVector(0.0f, 30.0f, 0.0f));

	InputFrame = CreateDefaultSubobject<USAOMMOInputFrameComponent>(TEXT("InputFrame"));
}

void ASAOVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InputFrame)
	{
		InputFrame->CurrentFrame.SourceDevice = ESAOInputDevice::VRController;
	}

	// In a full OpenXR build, the camera is driven by the HMD and the hands by
	// motion controllers. The transforms are mirrored into FSAOInputFrame by the
	// VR input provider so gameplay stays device-independent (Band 2 §4, §7).
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTarget(this);
	}
}
