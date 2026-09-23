// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOCharacterData.h"
#include "Engine/SkeletalMesh.h"

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