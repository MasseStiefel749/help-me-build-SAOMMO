// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOCharacterCreatorWidget.h"
#include "SAOCharacterData.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/AssetManager.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void USAOCharacterCreatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BodyTypeCombo)
	{
		BodyTypeCombo->ClearOptions();
		BodyTypeCombo->AddOption(TEXT("Manny (Male)"));
		BodyTypeCombo->AddOption(TEXT("Quinn (Female)"));
		BodyTypeCombo->OnSelectionChanged.AddDynamic(this, &USAOCharacterCreatorWidget::OnBodyTypeChanged);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnConfirmCharacter);
	}

	if (NameTextBox)
	{
		NameTextBox->OnTextCommitted.AddDynamic(this, &USAOCharacterCreatorWidget::OnNameCommitted);
	}

	// Color buttons
	if (SkinColorButton)
	{
		SkinColorButton->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnSkinColorChanged);
	}

	if (HairColorButton)
	{
		HairColorButton->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnHairColorChanged);
	}

	if (EyeColorButton)
	{
		EyeColorButton->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnEyeColorChanged);
	}

	PopulateArmorOptions();
	PopulateWeaponOptions();
	RefreshPreview();
}

void USAOCharacterCreatorWidget::InitializeCreator(USAOCharacterData* InCharacterData)
{
	CharacterData = InCharacterData;

	if (!CharacterData)
	{
		return;
	}

	// Initialize UI with current data
	if (BodyTypeCombo)
	{
		BodyTypeCombo->SetSelectedIndex(static_cast<int32>(CharacterData->Customization.BodyType));
	}

	if (SkinColorPreview)
	{
		SkinColorPreview->SetColorAndOpacity(CharacterData->Customization.SkinColor);
	}

	if (HairColorPreview)
	{
		HairColorPreview->SetColorAndOpacity(CharacterData->Customization.HairColor);
	}

	if (EyeColorPreview)
	{
		EyeColorPreview->SetColorAndOpacity(CharacterData->Customization.EyeColor);
	}

	if (NameTextBox)
	{
		NameTextBox->SetText(FText::FromString(CharacterData->Customization.CharacterName));
	}

	RefreshPreview();
}

void USAOCharacterCreatorWidget::OnBodyTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!CharacterData)
	{
		return;
	}

	CharacterData->Customization.BodyType = (SelectedItem.Contains(TEXT("Quinn"))) ? ESAOBodyType::Quinn : ESAOBodyType::Manny;
	RefreshPreview();
}

void USAOCharacterCreatorWidget::OnSkinColorChanged()
{
	if (CharacterData)
	{
		// Cycle through preset skin colors
		static const TArray<FLinearColor> SkinColors = {
			FLinearColor(1.0f, 0.85f, 0.7f),  // Light
			FLinearColor(0.8f, 0.65f, 0.5f),  // Medium
			FLinearColor(0.5f, 0.35f, 0.25f), // Dark
			FLinearColor(0.3f, 0.2f, 0.15f),  // Very dark
		};

		int32 CurrentIndex = SkinColors.IndexOfByKey(CharacterData->Customization.SkinColor);
		if (CurrentIndex == INDEX_NONE)
		{
			CurrentIndex = 0;
		}
		else
		{
			CurrentIndex = (CurrentIndex + 1) % SkinColors.Num();
		}

		CharacterData->Customization.SkinColor = SkinColors[CurrentIndex];

		if (SkinColorPreview)
		{
			SkinColorPreview->SetColorAndOpacity(CharacterData->Customization.SkinColor);
		}
	}
}

void USAOCharacterCreatorWidget::OnHairColorChanged()
{
	if (CharacterData)
	{
		static const TArray<FLinearColor> HairColors = {
			FLinearColor(0.3f, 0.2f, 0.1f),   // Dark brown
			FLinearColor(0.1f, 0.05f, 0.02f), // Black
			FLinearColor(0.6f, 0.4f, 0.2f),   // Blonde
			FLinearColor(0.7f, 0.3f, 0.1f),   // Red
			FLinearColor(0.9f, 0.8f, 0.6f),   // Platinum blonde
			FLinearColor(0.5f, 0.3f, 0.1f),   // Auburn
		};

		int32 CurrentIndex = HairColors.IndexOfByKey(CharacterData->Customization.HairColor);
		if (CurrentIndex == INDEX_NONE)
		{
			CurrentIndex = 0;
		}
		else
		{
			CurrentIndex = (CurrentIndex + 1) % HairColors.Num();
		}

		CharacterData->Customization.HairColor = HairColors[CurrentIndex];

		if (HairColorPreview)
		{
			HairColorPreview->SetColorAndOpacity(CharacterData->Customization.HairColor);
		}
	}
}

void USAOCharacterCreatorWidget::OnEyeColorChanged()
{
	if (CharacterData)
	{
		static const TArray<FLinearColor> EyeColors = {
			FLinearColor(0.2f, 0.4f, 0.6f),  // Blue
			FLinearColor(0.3f, 0.5f, 0.3f),  // Green
			FLinearColor(0.5f, 0.3f, 0.2f),  // Brown
			FLinearColor(0.6f, 0.5f, 0.3f),  // Hazel
			FLinearColor(0.8f, 0.2f, 0.2f),  // Red
			FLinearColor(0.5f, 0.5f, 0.5f),  // Gray
		};

		int32 CurrentIndex = EyeColors.IndexOfByKey(CharacterData->Customization.EyeColor);
		if (CurrentIndex == INDEX_NONE)
		{
			CurrentIndex = 0;
		}
		else
		{
			CurrentIndex = (CurrentIndex + 1) % EyeColors.Num();
		}

		CharacterData->Customization.EyeColor = EyeColors[CurrentIndex];

		if (EyeColorPreview)
		{
			EyeColorPreview->SetColorAndOpacity(CharacterData->Customization.EyeColor);
		}
	}
}

void USAOCharacterCreatorWidget::OnNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CharacterData)
	{
		CharacterData->Customization.CharacterName = Text.ToString();
	}
}

void USAOCharacterCreatorWidget::OnArmorSelected(ESAOArmorSlot ArmorSlot, TSoftObjectPtr<UStaticMesh> Mesh)
{
	if (!CharacterData)
	{
		return;
	}

	switch (ArmorSlot)
	{
	case ESAOArmorSlot::Helmet:
		CharacterData->Customization.HelmetMesh = Mesh;
		break;
	case ESAOArmorSlot::Chest:
		CharacterData->Customization.ChestMesh = Mesh;
		break;
	case ESAOArmorSlot::Coat:
		CharacterData->Customization.CoatMesh = Mesh;
		break;
	case ESAOArmorSlot::Boots:
		CharacterData->Customization.BootsMesh = Mesh;
		break;
	case ESAOArmorSlot::Gloves:
		CharacterData->Customization.GlovesMesh = Mesh;
		break;
	}
}

void USAOCharacterCreatorWidget::OnWeaponSelected(TSoftObjectPtr<UStaticMesh> Mesh)
{
	if (CharacterData)
	{
		CharacterData->Customization.WeaponMesh = Mesh;
	}
}

void USAOCharacterCreatorWidget::OnConfirmCharacter()
{
	// Save character data and start game
	if (CharacterData)
	{
		// Save to slot
		if (UGameplayStatics::DoesSaveGameExist(TEXT("CharacterSave"), 0))
		{
			// Overwrite existing
		}
		// Save via game instance or save system
		if (UGameplayStatics::SaveGameToSlot(CharacterData, TEXT("CharacterSave"), 0))
		{
			UE_LOG(LogTemp, Log, TEXT("Character saved successfully"));
		}

		// Open main level
		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::OpenLevel(World, TEXT("/Game/Levels/StartingReach/L_StartingReach"));
		}
	}
}

void USAOCharacterCreatorWidget::PopulateArmorOptions()
{
	if (!ArmorScrollBox)
	{
		return;
	}

	ArmorScrollBox->ClearChildren();

	// Helmet options
	TArray<FString> HelmetPaths = {
		TEXT("/Game/SAO/Armor/SK_Helm_Aincrad"),
		TEXT("/Game/SAO/Armor/SK_Helm_Aincrad_Variant"),
	};

	for (const FString& Path : HelmetPaths)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path))
		{
			UButton* Btn = NewObject<UButton>(this);
			Btn->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnHelmetButtonClicked);
			HelmetButtons.Add(Mesh);
			ArmorScrollBox->AddChild(Btn);
		}
	}

	// Chest options
	TArray<FString> ChestPaths = {
		TEXT("/Game/SAO/Armor/SK_Cuirass_Iron"),
		TEXT("/Game/SAO/Armor/SK_Cuirass_Leather"),
	};

	for (const FString& Path : ChestPaths)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path))
		{
			UButton* Btn = NewObject<UButton>(this);
			Btn->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnChestButtonClicked);
			ChestButtons.Add(Mesh);
			ArmorScrollBox->AddChild(Btn);
		}
	}

	// Coat options
	TArray<FString> CoatPaths = {
		TEXT("/Game/SAO/Armor/SK_Coat_Beta"),
	};

	for (const FString& Path : CoatPaths)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path))
		{
			UButton* Btn = NewObject<UButton>(this);
			Btn->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnCoatButtonClicked);
			CoatButtons.Add(Mesh);
			ArmorScrollBox->AddChild(Btn);
		}
	}
}

void USAOCharacterCreatorWidget::PopulateWeaponOptions()
{
	if (!WeaponScrollBox)
	{
		return;
	}

	WeaponScrollBox->ClearChildren();

	TArray<FString> WeaponPaths = {
		TEXT("/Game/SAO/Weapons/SM_BlackSword_ElucidatorLike"),
		TEXT("/Game/SAO/Weapons/SM_AzureBlade"),
		TEXT("/Game/SAO/Weapons/SM_StarterIronSword"),
		TEXT("/Game/SAO/Weapons/SM_Rapier_LambentLike"),
		TEXT("/Game/SAO/Weapons/SM_Dagger_Assassin"),
		TEXT("/Game/SAO/Weapons/SM_Greatsword_DragonBone"),
		TEXT("/Game/SAO/Weapons/SM_Shield_Kite"),
	};

	for (const FString& Path : WeaponPaths)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path))
		{
			UButton* Btn = NewObject<UButton>(this);
			Btn->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnWeaponButtonClicked);
			WeaponButtons.Add(Mesh);
			WeaponScrollBox->AddChild(Btn);
		}
	}
}

void USAOCharacterCreatorWidget::RefreshPreview()
{
	// Update preview image with current character setup
	// This would typically update a render target or character preview actor
}

void USAOCharacterCreatorWidget::OnHelmetButtonClicked()
{
	if (!CharacterData || HelmetButtons.IsEmpty())
	{
		return;
	}

	// Cycle through helmet options
	static int32 HelmetIndex = 0;
	HelmetIndex = (HelmetIndex + 1) % HelmetButtons.Num();
	
	if (HelmetButtons.IsValidIndex(HelmetIndex))
	{
		OnArmorSelected(ESAOArmorSlot::Helmet, HelmetButtons[HelmetIndex]);
		RefreshPreview();
	}
}

void USAOCharacterCreatorWidget::OnChestButtonClicked()
{
	if (!CharacterData || ChestButtons.IsEmpty())
	{
		return;
	}

	static int32 ChestIndex = 0;
	ChestIndex = (ChestIndex + 1) % ChestButtons.Num();
	
	if (ChestButtons.IsValidIndex(ChestIndex))
	{
		OnArmorSelected(ESAOArmorSlot::Chest, ChestButtons[ChestIndex]);
		RefreshPreview();
	}
}

void USAOCharacterCreatorWidget::OnCoatButtonClicked()
{
	if (!CharacterData || CoatButtons.IsEmpty())
	{
		return;
	}

	static int32 CoatIndex = 0;
	CoatIndex = (CoatIndex + 1) % CoatButtons.Num();
	
	if (CoatButtons.IsValidIndex(CoatIndex))
	{
		OnArmorSelected(ESAOArmorSlot::Coat, CoatButtons[CoatIndex]);
		RefreshPreview();
	}
}

void USAOCharacterCreatorWidget::OnWeaponButtonClicked()
{
	if (!CharacterData || WeaponButtons.IsEmpty())
	{
		return;
	}

	static int32 WeaponIndex = 0;
	WeaponIndex = (WeaponIndex + 1) % WeaponButtons.Num();
	
	if (WeaponButtons.IsValidIndex(WeaponIndex))
	{
		OnWeaponSelected(WeaponButtons[WeaponIndex]);
		RefreshPreview();
	}
}