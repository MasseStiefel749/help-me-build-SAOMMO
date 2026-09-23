// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameWorldSubsystem.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "WorldPartition/DataLayer/WorldDataLayers.h"

void UGameWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

AWorldDataLayers* UGameWorldSubsystem::RepairLevelDataLayers(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World || !World->PersistentLevel)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWorldDataLayers* Fresh = World->SpawnActor<AWorldDataLayers>(
		AWorldDataLayers::StaticClass(), FTransform::Identity, Params);
	if (!Fresh)
	{
		return nullptr;
	}

	World->PersistentLevel->SetWorldDataLayers(Fresh);
	return Fresh;
}
