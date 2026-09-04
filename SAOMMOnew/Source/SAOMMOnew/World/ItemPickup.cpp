// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "InventoryComponent.h"

AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;

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

	// Sensible blockout default so a freshly placed pickup works with no setup.
	Item.ItemId = FName(TEXT("HealthHerb"));
	Item.DisplayName = FText::FromString(TEXT("Health Herb"));
	Item.Type = EItemType::Consumable;
	Item.Count = 2;

	PickupVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnPickupOverlap);
}

void AItemPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only pawns with somewhere to put it (avoids enemies vacuuming loot).
	if (OtherActor && OtherActor->FindComponentByClass<UInventoryComponent>())
	{
		TryPickup(OtherActor);
	}
}

void AItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Gentle spin so pickups read as interactive, not scenery.
	if (Mesh)
	{
		Mesh->AddLocalRotation(FRotator(0.0f, 60.0f * DeltaTime, 0.0f));
	}
}

void AItemPickup::Configure(FName ItemId, int32 Count)
{
	if (!ItemId.IsNone() && Count > 0)
	{
		Item.ItemId = ItemId;
		Item.Count = Count;
	}
}

bool AItemPickup::TryPickup(AActor* Caller)
{
	if (!Caller || Item.ItemId.IsNone() || Item.Count <= 0)
	{
		return false;
	}

	UInventoryComponent* Inventory = Caller->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		if (APawn* Pawn = Cast<APawn>(Caller))
		{
			// Allow pickups resolved from a controller-owned pawn as well.
			if (AController* Controller = Pawn->GetController())
			{
				if (APawn* Controlled = Controller->GetPawn())
				{
					Inventory = Controlled->FindComponentByClass<UInventoryComponent>();
				}
			}
		}
	}
	if (!Inventory)
	{
		return false;
	}

	Inventory->AddItem(Item);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green,
			FString::Printf(TEXT("Picked up: %s x%d"), *Item.DisplayName.ToString(), Item.Count));
	}
	Destroy();
	return true;
}
