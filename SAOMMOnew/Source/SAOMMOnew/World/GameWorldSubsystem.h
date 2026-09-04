// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameWorldSubsystem.generated.h"

/**
 *  World subsystem stub (Band 2 §12, Band 4).
 *
 *  The full world (regions, streaming, World Partition) is deferred until
 *  gameplay is proven. This minimal subsystem provides a shared, world-scoped
 *  place to track the active region and future world state without pulling in
 *  heavy systems prematurely.
 */
UCLASS()
class UGameWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Name of the region the player is currently in (provisional, Band 4 §17). */
	UFUNCTION(BlueprintCallable, Category = "World")
	FString GetCurrentRegion() const { return CurrentRegion; }

	UFUNCTION(BlueprintCallable, Category = "World")
	void SetCurrentRegion(const FString& RegionName) { CurrentRegion = RegionName; }

protected:

	FString CurrentRegion = TEXT("Starting Reach");
};
