// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOCharacterData.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

USAOCharacterData::USAOCharacterData()
{
}

void USAOCharacterData::ApplyToMesh(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	// Set body mesh
	USkeletalMesh* BodyMesh = GetBodyMesh();
	if (BodyMesh)
	{
		MeshComp->SetSkeletalMesh(BodyMesh);
	}

	// Skin/material tint: material parameter names vary between the mannequin
	// materials, so try the known candidates - setting a parameter a material
	// doesn't expose is a harmless no-op on a MID. Hair/eye colors stay saved
	// until assets with dedicated slots exist (mannequin is bald/smooth).
	if (UMaterialInstanceDynamic* MID = MeshComp->CreateDynamicMaterialInstance(0))
	{
		static const FName TintCandidates[] = {
			TEXT("Skin Color"), TEXT("SkinColor"), TEXT("Skin Tint"), TEXT("SkinTint"),
			TEXT("Base Color"), TEXT("BaseColor"), TEXT("Tint"), TEXT("Color"),
		};
		for (const FName& Param : TintCandidates)
		{
			MID->SetVectorParameterValue(Param, Customization.SkinColor);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("Character appearance applied (body=%s)"),
		Customization.BodyType == ESAOBodyType::Quinn ? TEXT("Quinn") : TEXT("Manny"));
}

USkeletalMesh* USAOCharacterData::GetBodyMesh() const
{
	switch (Customization.BodyType)
	{
	case ESAOBodyType::Quinn:
		return LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	case ESAOBodyType::Manny:
	default:
		return LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	}
}

TSubclassOf<UAnimInstance> USAOCharacterData::GetAnimClass() const
{
	switch (Customization.BodyType)
	{
	case ESAOBodyType::Quinn:
		return LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	case ESAOBodyType::Manny:
	default:
		return LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	}
}