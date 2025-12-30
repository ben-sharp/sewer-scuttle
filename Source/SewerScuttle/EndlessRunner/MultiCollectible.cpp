// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiCollectible.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "RabbitCharacter.h"
#include "CurrencyManager.h"
#include "EndlessRunnerGameMode.h"

AMultiCollectible::AMultiCollectible()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root scene component (no collision)
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;
	
	// Ensure root has no collision
	if (UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(RootComponent))
	{
		PrimitiveRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMultiCollectible::BeginPlay()
{
	Super::BeginPlay();

	// Setup items from child components
	SetupItems();
}

void AMultiCollectible::SetupItems()
{
	CollectibleItems.Empty();
	CollisionToItemMap.Empty();

	// Find all child static mesh components and sphere components
	TArray<UActorComponent*> Components;
	GetComponents(UStaticMeshComponent::StaticClass(), Components);

	// Group mesh and collision components by naming convention or hierarchy
	// Look for pairs: MeshComponent_0, CollisionSphere_0, MeshComponent_1, CollisionSphere_1, etc.
	TMap<int32, FCollectibleItem> ItemMap;

	for (UActorComponent* Comp : Components)
	{
		if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Comp))
		{
			FString CompName = MeshComp->GetName();
			
			// Try to extract index from name (e.g., "MeshComponent_0" -> 0)
			int32 ItemIndex = 0;
			if (CompName.Contains(TEXT("_")))
			{
				FString IndexStr = CompName.RightChop(CompName.Find(TEXT("_")) + 1);
				ItemIndex = FCString::Atoi(*IndexStr);
			}

			// Ensure mesh has no collision (only visual, collision handled by sphere)
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			// Find or create item
			if (!ItemMap.Contains(ItemIndex))
			{
				FCollectibleItem NewItem;
				NewItem.MeshComponent = MeshComp;
				ItemMap.Add(ItemIndex, NewItem);
			}
			else
			{
				ItemMap[ItemIndex].MeshComponent = MeshComp;
			}
		}
	}

	// Find collision spheres
	TArray<UActorComponent*> CollisionComponents;
	GetComponents(USphereComponent::StaticClass(), CollisionComponents);

	for (UActorComponent* Comp : CollisionComponents)
	{
		if (USphereComponent* SphereComp = Cast<USphereComponent>(Comp))
		{
			FString CompName = SphereComp->GetName();
			
			// Try to extract index from name
			int32 ItemIndex = 0;
			if (CompName.Contains(TEXT("_")))
			{
				FString IndexStr = CompName.RightChop(CompName.Find(TEXT("_")) + 1);
				ItemIndex = FCString::Atoi(*IndexStr);
			}

			// Setup collision (only if radius is still at default/unset)
			// This allows Blueprint customization to persist
			if (SphereComp->GetUnscaledSphereRadius() < 1.0f || FMath::IsNearlyEqual(SphereComp->GetUnscaledSphereRadius(), 0.0f, 0.1f))
			{
				SphereComp->SetSphereRadius(50.0f);
			}
			SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SphereComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
			SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMultiCollectible::OnItemOverlapBegin);

			// Add to item or create new
			if (ItemMap.Contains(ItemIndex))
			{
				ItemMap[ItemIndex].CollisionSphere = SphereComp;
			}
			else
			{
				FCollectibleItem NewItem;
				NewItem.CollisionSphere = SphereComp;
				ItemMap.Add(ItemIndex, NewItem);
			}

			// Map collision to item index
			CollisionToItemMap.Add(SphereComp, ItemIndex);
		}
	}

	// Convert map to array (sorted by index)
	TArray<int32> Indices;
	ItemMap.GetKeys(Indices);
	Indices.Sort();

	for (int32 Index : Indices)
	{
		FCollectibleItem& Item = ItemMap[Index];
		
		// Set value from ItemValues array, or use default
		if (Index < ItemValues.Num() && ItemValues[Index] > 0)
		{
			Item.Value = ItemValues[Index];
		}
		else
		{
			Item.Value = DefaultItemValue;
		}
		
		// Store initial positions for animation
		if (Item.MeshComponent)
		{
			Item.InitialZ = Item.MeshComponent->GetComponentLocation().Z;
			Item.InitialRotation = Item.MeshComponent->GetComponentRotation();
		}
		else if (Item.CollisionSphere)
		{
			Item.InitialZ = Item.CollisionSphere->GetComponentLocation().Z;
		}

		CollectibleItems.Add(Item);
	}

	UE_LOG(LogTemp, Log, TEXT("MultiCollectible: Setup %d collectible items"), CollectibleItems.Num());
}

void AMultiCollectible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAnimateItems || CollectibleItems.Num() == 0)
	{
		return;
	}

	// Animate all uncollected items
	for (FCollectibleItem& Item : CollectibleItems)
	{
		if (Item.bCollected || !Item.MeshComponent)
		{
			continue;
		}

		// Rotate
		FRotator CurrentRotation = Item.MeshComponent->GetComponentRotation();
		CurrentRotation.Yaw += RotationSpeed * DeltaTime;
		Item.MeshComponent->SetWorldRotation(CurrentRotation);

		// Bob
		float NewZ = Item.InitialZ + FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) * BobAmplitude;
		FVector Location = Item.MeshComponent->GetComponentLocation();
		Location.Z = NewZ;
		Item.MeshComponent->SetWorldLocation(Location);
	}
}

void AMultiCollectible::OnItemOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// Only respond to the player's capsule component to prevent multiple overlap events
	// The character's capsule is the main collision component
	if (OtherComp != Player->GetCapsuleComponent())
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("MultiCollectible: Ignoring overlap - not capsule component (Component: %s)"), *OtherComp->GetName());
		return;
	}

	// Find which item was overlapped
	if (USphereComponent* SphereComp = Cast<USphereComponent>(OverlappedComp))
	{
		if (int32* ItemIndexPtr = CollisionToItemMap.Find(SphereComp))
		{
			int32 ItemIndex = *ItemIndexPtr;
			// Check if already collected before calling CollectItem
			if (ItemIndex >= 0 && ItemIndex < CollectibleItems.Num() && !CollectibleItems[ItemIndex].bCollected)
			{
				// Disable collision IMMEDIATELY to prevent multiple overlap events
				// This must happen before CollectItem() to prevent race conditions
				if (CollectibleItems[ItemIndex].CollisionSphere)
				{
					CollectibleItems[ItemIndex].CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					// Also remove the overlap delegate to be extra safe
					CollectibleItems[ItemIndex].CollisionSphere->OnComponentBeginOverlap.RemoveAll(this);
				}
				
				UE_LOG(LogTemp, Log, TEXT("MultiCollectible: Collecting item %d (Value: %d, Actor: %s)"), ItemIndex, CollectibleItems[ItemIndex].Value, *GetName());
				CollectItem(ItemIndex);
			}
			else if (ItemIndex >= 0 && ItemIndex < CollectibleItems.Num() && CollectibleItems[ItemIndex].bCollected)
			{
				UE_LOG(LogTemp, VeryVerbose, TEXT("MultiCollectible: Ignoring overlap - item %d already collected"), ItemIndex);
			}
		}
	}
}

void AMultiCollectible::CollectItem(int32 ItemIndex)
{
	if (ItemIndex < 0 || ItemIndex >= CollectibleItems.Num())
	{
		return;
	}

	FCollectibleItem& Item = CollectibleItems[ItemIndex];
	if (Item.bCollected)
	{
		return;
	}

	Item.bCollected = true;

	// Hide mesh
	if (Item.MeshComponent)
	{
		Item.MeshComponent->SetVisibility(false);
	}

	// Disable collision
	if (Item.CollisionSphere)
	{
		Item.CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Play collection effect
	if (CollectionEffect)
	{
		FVector EffectLocation = Item.MeshComponent ? Item.MeshComponent->GetComponentLocation() : GetActorLocation();
		if (UParticleSystemComponent* EffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CollectionEffect, EffectLocation))
		{
			// Effect will auto-destroy
		}
	}

	// Calculate final value (apply special multiplier if special collectible)
	int32 FinalValue = Item.Value;
	if (bIsSpecial)
	{
		FinalValue = FMath::FloorToInt(Item.Value * SpecialValueMultiplier);
		UE_LOG(LogTemp, Log, TEXT("MultiCollectible: Special collectible item - Value %d * %.2f = %d"), Item.Value, SpecialValueMultiplier, FinalValue);
	}

	// Add to run currency (temporary, resets each game)
	if (UWorld* World = GetWorld())
	{
		if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
		{
			GameMode->AddRunCurrency(FinalValue);
		}
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("MultiCollectible: Collected item %d (value: %d)"), ItemIndex, Item.Value);

	// Check if all items are collected
	if (GetUncollectedCount() == 0)
	{
		// All items collected, destroy actor after a delay
		SetLifeSpan(1.0f);
	}
}

int32 AMultiCollectible::GetTotalValue() const
{
	int32 Total = 0;
	for (const FCollectibleItem& Item : CollectibleItems)
	{
		if (!Item.bCollected)
		{
			Total += Item.Value;
		}
	}
	return Total;
}

int32 AMultiCollectible::GetUncollectedCount() const
{
	int32 Count = 0;
	for (const FCollectibleItem& Item : CollectibleItems)
	{
		if (!Item.bCollected)
		{
			Count++;
		}
	}
	return Count;
}

void AMultiCollectible::CollectAll()
{
	for (int32 i = 0; i < CollectibleItems.Num(); ++i)
	{
		if (!CollectibleItems[i].bCollected)
		{
			CollectItem(i);
		}
	}
}

