// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SAOCharacterData.h"
#include "SAOCharacterCreatorWidget.generated.h"

class UButton;
class UComboBoxString;
class UTextBlock;
class UImage;
class UEditableTextBox;
class UScrollBox;
class UProgressBar;

/**
 *  Character creator widget shown at game start.
 *  Allows player to customize their character appearance and equipment.
 */
UCLASS(Blueprintable)
class USAOCharacterCreatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void InitializeCreator(USAOCharacterData* InCharacterData);

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnConfirmCharacter();

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnBodyTypeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnSkinColorChanged();

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnHairColorChanged();

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnEyeColorChanged();

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnNameCommitted(const FText& Text, ETextCommit::Type CommitType);

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnArmorSelected(ESAOArmorSlot ArmorSlot, TSoftObjectPtr<UStaticMesh> Mesh);

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void OnWeaponSelected(TSoftObjectPtr<UStaticMesh> Mesh);

	// Button click handlers for armor/weapon selection
	UFUNCTION()
	void OnHelmetButtonClicked();

	UFUNCTION()
	void OnChestButtonClicked();

	UFUNCTION()
	void OnCoatButtonClicked();

	UFUNCTION()
	void OnWeaponButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Character Creator")
	void RefreshPreview();
protected:
	void PopulateArmorOptions();
	void PopulateWeaponOptions();
	void RefreshInventoryDisplay();
private:
	UPROPERTY()
	TObjectPtr<USAOCharacterData> CharacterData;

	// UI Elements
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> BodyTypeCombo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkinColorButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HairColorButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EyeColorButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SkinColorPreview;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HairColorPreview;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> EyeColorPreview;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NameTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PreviewImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ArmorScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WeaponScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButtonBottom;

	// Armor slots
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HelmetSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChestSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CoatSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BootsSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GlovesSlot;

	// Weapon slots
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OffHandSlot;

	// Preview
	UPROPERTY()
	TObjectPtr<UImage> CharacterPreviewImage;

	// Button arrays for dynamic creation
	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> HelmetButtons;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> ChestButtons;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> CoatButtons;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMesh>> WeaponButtons;
};
