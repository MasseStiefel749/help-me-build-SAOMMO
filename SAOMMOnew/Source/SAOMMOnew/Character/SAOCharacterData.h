// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SAOCharacterData.generated.h"

class USkeletalMesh;
class USkeletalMeshComponent;
class UAnimInstance;
class UStaticMesh;

/** Body type preset (uses existing UE5 mannequins) */
UENUM(BlueprintType)
enum class ESAOBodyType : uint8
{
	Manny    UMETA(DisplayName = "Manny (Male)"),
	Quinn    UMETA(DisplayName = "Quinn (Female)"),
};

/** Armor slot for equipment */
UENUM(BlueprintType)
enum class ESAOArmorSlot : uint8
{
	Helmet,
	Chest,
	Coat,
	Boots,
	Gloves,
};

/** Character customization data - saved/loaded per player */
USTRUCT(BlueprintType)
struct FSAOCharacterCustomization
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ESAOBodyType BodyType = ESAOBodyType::Manny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FLinearColor SkinColor = FLinearColor(1.0f, 0.85f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FLinearColor EyeColor = FLinearColor(0.2f, 0.4f, 0.6f);

	// Armor pieces (references to static meshes in Content/SAO/Armor/)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> HelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> CoatMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> BootsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> GlovesMesh;

	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UStaticMesh> WeaponMesh;

	// Starting stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttack = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDefense = 5.0f;

	// Character name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CharacterName = TEXT("Hero");
};

UCLASS(BlueprintType, Blueprintable)
class USAOCharacterData : public UObject
{
	GENERATED_BODY()

public:
	USAOCharacterData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FSAOCharacterCustomization Customization;

	/** Applies this customization to a skeletal mesh component */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void ApplyToMesh(USkeletalMeshComponent* MeshComp) const;

	/** Returns the body skeletal mesh for the current body type */
	UFUNCTION(BlueprintCallable, Category = "Character")
	USkeletalMesh* GetBodyMesh() const;

	/** Returns the anim instance class for the current body type */
	UFUNCTION(BlueprintCallable, Category = "Character")
	TSubclassOf<UAnimInstance> GetAnimClass() const;
};