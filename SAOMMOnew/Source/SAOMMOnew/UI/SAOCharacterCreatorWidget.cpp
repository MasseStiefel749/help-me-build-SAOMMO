// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOCharacterCreatorWidget.h"
#include "SAOCharacterData.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"

void USAOCharacterCreatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Backlog #18(b): the root widget must support keyboard focus, otherwise
	// FInputModeUIOnly::SetWidgetToFocus logs "Attempting to focus
	// Non-Focusable widget" (PlayerController.cpp:6345) every time the
	// controller opens the creator. NativeConstruct runs during AddToViewport,
	// i.e. before MainPlayerController sets the input mode.
	SetIsFocusable(true);

	// Build the whole layout in code when no Blueprint designer tree exists,
	// so the creator is usable with zero Editor assets (HUD pattern).
	BuildLayoutIfNeeded();

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

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &USAOCharacterCreatorWidget::OnCancelCharacter);
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

	// Data may have been set before the layout existed (init-before-construct
	// ordering); push it into the freshly built controls.
	if (CharacterData)
	{
		ApplyDataToControls();
	}

	RefreshPreview();
}

void USAOCharacterCreatorWidget::InitializeCreator(USAOCharacterData* InCharacterData)
{
	CharacterData = InCharacterData;
	ApplyDataToControls();
}

void USAOCharacterCreatorWidget::ApplyDataToControls()
{
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
		// Save via game instance or save system
		if (UGameplayStatics::SaveGameToSlot(CharacterData, TEXT("CharacterSave"), 0))
		{
			UE_LOG(LogTemp, Log, TEXT("Character saved successfully"));
		}

		// Open main level (unpause first - the overlay froze the world).
		if (UWorld* World = GetWorld())
		{
			UGameplayStatics::SetGamePaused(World, false);
			UGameplayStatics::OpenLevel(World, TEXT("/Game/Levels/StartingReach/L_StartingReach"));
		}
	}
}

void USAOCharacterCreatorWidget::OnCancelCharacter()
{
	// Close without saving: restore gameplay input and unpause.
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	RemoveFromParent();
}

void USAOCharacterCreatorWidget::PopulateArmorOptions()
{
	if (!ArmorScrollBox)
	{
		return;
	}

	ArmorScrollBox->ClearChildren();

	// No armor meshes exist in the project yet (the old /Game/SAO/... paths
	// pointed at a folder that was never created, so this section always
	// loaded nothing). Keep it honestly empty and add entries here once
	// licensed armor assets land under Content/Armor/.
}

void USAOCharacterCreatorWidget::PopulateWeaponOptions()
{
	if (!WeaponScrollBox)
	{
		return;
	}

	WeaponScrollBox->ClearChildren();

	// The one weapon mesh that exists: project-authored, imported sword.
	TArray<FString> WeaponPaths = {
		TEXT("/Game/Weapons/SM_SAOSword"),
	};

	for (const FString& Path : WeaponPaths)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path))
		{
			UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
			UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Caption->SetText(FText::FromString(Mesh->GetName()));
			Btn->SetContent(Caption);
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

void USAOCharacterCreatorWidget::BuildLayoutIfNeeded()
{
	// Code-only layout: only runs when no designer tree exists (no Blueprint
	// asset yet), same zero-Editor-setup pattern as the HUD and death screen.
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	auto MakeLabel = [this](const FString& Text) -> UTextBlock*
	{
		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetText(FText::FromString(Text));
		Label->SetColorAndOpacity(FLinearColor(0.88f, 0.88f, 0.88f, 1.0f));
		return Label;
	};

	auto MakeButton = [this](const FString& Text) -> UButton*
	{
		UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Caption = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Caption->SetText(FText::FromString(Text));
		Btn->SetContent(Caption);
		return Btn;
	};

	auto MakeSwatch = [this](const FName& Name) -> UImage*
	{
		UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(FLinearColor::White);
		Img->SetBrush(Brush);
		Img->SetDesiredSizeOverride(FVector2D(72.0f, 24.0f));
		return Img;
	};

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CreatorCanvas"));
	WidgetTree->RootWidget = Canvas;

	// Dimmer behind a centered panel.
	UBorder* Dimmer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CreatorDimmer"));
	Dimmer->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.05f, 0.92f));

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CreatorPanel"));
	Panel->SetBrushColor(FLinearColor(0.07f, 0.07f, 0.11f, 0.97f));
	Panel->SetPadding(FMargin(28.0f));

	UVerticalBox* CenterBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CreatorBox"));
	Panel->SetContent(CenterBox);

	UTextBlock* Title = MakeLabel(TEXT("CHARACTER CREATOR"));
	Title->SetColorAndOpacity(FLinearColor(0.92f, 0.78f, 0.30f, 1.0f));
	CenterBox->AddChildToVerticalBox(Title);

	auto AddRow = [&](UWidget* Left, UWidget* Right)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (UHorizontalBoxSlot* LeftSlot = Row->AddChildToHorizontalBox(Left))
		{
			LeftSlot->SetVerticalAlignment(VAlign_Center);
		}
		if (UHorizontalBoxSlot* RightSlot = Row->AddChildToHorizontalBox(Right))
		{
			RightSlot->SetVerticalAlignment(VAlign_Center);
		}
		if (UVerticalBoxSlot* RowSlot = CenterBox->AddChildToVerticalBox(Row))
		{
			RowSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 8.0f));
		}
	};

	NameTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CreatorName"));
	NameTextBox->SetHintText(FText::FromString(TEXT("Hero")));
	AddRow(MakeLabel(TEXT("Name")), NameTextBox);

	BodyTypeCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("CreatorBodyType"));
	AddRow(MakeLabel(TEXT("Body")), BodyTypeCombo);

	SkinColorButton = MakeButton(TEXT("Skin color"));
	SkinColorPreview = MakeSwatch(TEXT("SkinSwatch"));
	AddRow(SkinColorButton, SkinColorPreview);

	HairColorButton = MakeButton(TEXT("Hair color"));
	HairColorPreview = MakeSwatch(TEXT("HairSwatch"));
	AddRow(HairColorButton, HairColorPreview);

	EyeColorButton = MakeButton(TEXT("Eye color"));
	EyeColorPreview = MakeSwatch(TEXT("EyeSwatch"));
	AddRow(EyeColorButton, EyeColorPreview);

	// Buttons: a sword preview/weapon list only appears when a Blueprint
	// provides the scroll boxes; the core creator is name/body/colors.
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CreatorButtons"));
	ConfirmButton = MakeButton(TEXT("CONFIRM"));
	CancelButton = MakeButton(TEXT("CANCEL"));
	if (UHorizontalBoxSlot* ConfirmSlot = Buttons->AddChildToHorizontalBox(ConfirmButton))
	{
		ConfirmSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}
	Buttons->AddChildToHorizontalBox(CancelButton);
	if (UVerticalBoxSlot* ButtonRowSlot = CenterBox->AddChildToVerticalBox(Buttons))
	{
		ButtonRowSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
	}

	if (UCanvasPanelSlot* DimSlot = Canvas->AddChildToCanvas(Dimmer))
	{
		DimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		DimSlot->SetOffsets(FMargin(0.0f));
	}
	if (UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetOffsets(FMargin(0.0f));
	}
}