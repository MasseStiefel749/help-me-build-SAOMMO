// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAOItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "SAOMMOInventoryComponent.h"

ASAOItemPickup::ASAOItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupVolume = CreateDefaultSubobject<USphereComponent>(TEXT("PickupVolume"));
	RootComponent = PickupVolume;
	PickupVolume->SetSphereRadius(PickupRadius);
	PickupVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(PickupVolume);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ASAOItemPickup::TryPickup(AActor* Caller)
{
	if (!Caller || Item.ItemId.IsNone() || Item.Count <= 0)
	{
		return false;
	}

	USAOMMOInventoryComponent* Inventory = Caller->FindComponentByClass<USAOMMOInventoryComponent>();
	if (!Inventory)
	{
		if (APawn* Pawn = Cast<APawn>(Caller))
		{
			// Allow pickups resolved from a controller-owned pawn as well.
			if (AController* Controller = Pawn->GetController())
			{
				if (APawn* Controlled = Controller->GetPawn())
				{
					Inventory = Controlled->FindComponentByClass<USAOMMOInventoryComponent>();
				}
			}
		}
	}
	if (!Inventory)
	{
		return false;
	}

	Inventory->AddItem(Item);
	Destroy();
	return true;
}
