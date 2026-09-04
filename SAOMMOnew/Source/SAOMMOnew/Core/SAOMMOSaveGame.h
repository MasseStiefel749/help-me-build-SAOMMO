// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SAOItemTypes.h"
#include "SAOMMOSaveGame.generated.h"

/**
 *  Built-in save wrapper (Band 2 §15).
 *
 *  Uses Unreal's USaveGame rather than a custom framework, per the architecture
 *  decision to keep the first save implementation simple. A custom save system
 *  is only introduced if USaveGame proves insufficient.
 */
UCLASS()
class USAOMMOSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	/** Save slot identifier. */
	UPROPERTY(VisibleAnywhere, Category = "Save")
	FString SlotName = TEXT("SAOMMO_Save");

	/** User index for multiple local players. */
	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 UserIndex = 0;

	/** Last saved player location. */
	UPROPERTY(EditAnywhere, Category = "Save")
	FVector PlayerLocation = FVector::ZeroVector;

	/** Last saved player rotation. */
	UPROPERTY(EditAnywhere, Category = "Save")
	FRotator PlayerRotation = FRotator::ZeroRotator;

	/** Persisted player health. */
	UPROPERTY(EditAnywhere, Category = "Save")
	float PlayerHealth = 0.0f;

	/** Persisted inventory. */
	UPROPERTY(EditAnywhere, Category = "Save")
	TArray<FSAOItem> Inventory;

	/** Persisted progression. */
	UPROPERTY(EditAnywhere, Category = "Save")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Save")
	float Experience = 0.0f;
};
