// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "MotionControllerComponent.h"
#include "Sword.h"
#include "CombatComponent.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->bUsePawnControlRotation = false;

	// Tracked hand anchors (Band 2 §7, audit P6): motion controllers bound to
	// the standard OpenXR grip poses. Without tracking they keep these offsets
	// (the component falls back to its current relative transform).
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(VROrigin);
	LeftHand->SetTrackingMotionSource(FName(TEXT("Left")));
	LeftHand->SetRelativeLocation(FVector(0.0f, -30.0f, 0.0f));

	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(VROrigin);
	RightHand->SetTrackingMotionSource(FName(TEXT("Right")));
	RightHand->SetRelativeLocation(FVector(0.0f, 30.0f, 0.0f));

	InputFrame = CreateDefaultSubobject<UInputFrameComponent>(TEXT("InputFrame"));

	// Combat (Band 2 §8): arms the sword from swing velocity — the gate only
	// counts velocity on motion devices, so this is the designed VR feed
	// (audit P8; the component itself is not modified).
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	DefaultSwordClass = ASword::StaticClass();
}

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InputFrame)
	{
		InputFrame->CurrentFrame.SourceDevice = EInputDevice::VRController;
	}

	// Band 3 §20 P7 ("Hold a sword"), audit G4: spawn the sword and bind it to
	// the tracked right hand — in VR the motion controller IS the hand, so no
	// skeleton socket exists. Grip alignment against the controller axes still
	// needs a headset pass (proof item, backlog #8).
	if (DefaultSwordClass && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EquippedSword = GetWorld()->SpawnActor<ASword>(DefaultSwordClass, GetActorTransform(), Params);
		if (EquippedSword)
		{
			EquippedSword->AttachToComponent(
				RightHand ? static_cast<USceneComponent*>(RightHand) : GetRootComponent(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			// The VR pawn renders no body: a floating sword shadow would read as
			// a bug (same reasoning as the desktop first-person path, Sword.h).
			EquippedSword->SetShadowCasting(false);
			EquippedSword->SetOwnerActor(this);
			if (CombatComponent)
			{
				CombatComponent->SetSword(EquippedSword);
			}
		}
	}

	// With a full OpenXR build the camera is driven by the HMD and the hands by
	// motion controllers; Tick mirrors those transforms into FInputFrame so
	// gameplay stays device-independent (Band 2 §4, §7).
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTarget(this);
	}
}

void AVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Mirror the tracked HMD/hand poses into the device-independent frame
	// (Band 2 §3/§4): gameplay reads FInputFrame, never OpenXR types. Poses are
	// world space; on non-VR devices FInputFrame::Reset() keeps them neutral.
	if (InputFrame && LeftHand && RightHand && VRCamera)
	{
		FInputFrame& Frame = InputFrame->CurrentFrame;
		Frame.HeadRotation = VRCamera->GetComponentRotation();
		Frame.LeftHandPosition = LeftHand->GetComponentLocation();
		Frame.LeftHandRotation = LeftHand->GetComponentRotation();
		Frame.RightHandPosition = RightHand->GetComponentLocation();
		Frame.RightHandRotation = RightHand->GetComponentRotation();
	}
}

bool AVRCharacter::IsXRSessionActive()
{
	// Single XR activation gate (ADR-017c): GEngine->XRSystem (Engine.h) is the
	// registered IXRTrackingSystem — MainGameMode's initial pawn choice and
	// MainPlayerController's respawn both ask this one question.
	return GEngine && GEngine->XRSystem.IsValid();
}
