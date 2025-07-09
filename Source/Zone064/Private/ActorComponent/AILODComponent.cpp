// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/AILODComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAILODComponent::UAILODComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAILODComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AILODComponent owner is not a Pawn."));
		SetComponentTickEnabled(false);
		return;
	}

	// Sort settings by distance, descending.
	LODSettings.Sort([](const FAILODSetting& A, const FAILODSetting& B) {
		return A.DistanceThreshold > B.DistanceThreshold;
	});

	// Force an initial LOD check
	TickComponent(0.0f, ELevelTick::LEVELTICK_All, nullptr);
}

void UAILODComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPawn || LODSettings.Num() == 0)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const float DistanceSquared = FVector::DistSquared(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

	EAILODLevel NewLODLevel = EAILODLevel::High; // Default to high
	int32 LODIndex = LODSettings.Num() - 1;

	for (int32 i = 0; i < LODSettings.Num(); ++i)
	{
		if (DistanceSquared > FMath::Square(LODSettings[i].DistanceThreshold))
		{
			LODIndex = i;
			break;
		}
	}
	
	NewLODLevel = static_cast<EAILODLevel>(LODIndex);

	if (NewLODLevel != CurrentLODLevel)
	{
		CurrentLODLevel = NewLODLevel;
		ApplyLODSettings(CurrentLODLevel);
	}
}

void UAILODComponent::ApplyLODSettings(EAILODLevel NewLODLevel)
{
	if (!OwnerPawn || !LODSettings.IsValidIndex(static_cast<int32>(NewLODLevel)))
	{
		return;
	}

	const FAILODSetting& CurrentSetting = LODSettings[static_cast<int32>(NewLODLevel)];
	OwnerPawn->SetActorTickInterval(CurrentSetting.TickInterval);

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	UCharacterMovementComponent* MovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();

	// Example: Adjust properties based on LOD
	switch (NewLODLevel)
	{
	case EAILODLevel::High:
		// Full functionality
		if (AIController)
		{
			if (auto Perception = AIController->GetAIPerceptionComponent())
			{
				Perception->Activate();
			}
		}
		if (MovementComponent)
		{
			// Set high-detail movement properties
		}
		OwnerPawn->SetActorHiddenInGame(false);
		break;

	case EAILODLevel::Medium:
		if (AIController)
		{
			if (auto Perception = AIController->GetAIPerceptionComponent())
			{
				Perception->Activate();
			}
		}
		if (MovementComponent)
        {
			// Set medium-detail movement properties
        }
		OwnerPawn->SetActorHiddenInGame(false);
		break;

	case EAILODLevel::Low:
		// Reduced functionality
		if (AIController)
		{
			if (auto Perception = AIController->GetAIPerceptionComponent())
			{
				Perception->Deactivate();
			}
		}
		if (MovementComponent)
		{
			// Set low-detail movement properties, e.g., lower simulation quality
		}
		OwnerPawn->SetActorHiddenInGame(false);
		break;

	case EAILODLevel::Culled:
		// Minimal functionality, possibly hidden
		if (AIController)
		{
			AIController->StopMovement();
			if (auto Perception = AIController->GetAIPerceptionComponent())
			{
				Perception->Deactivate();
			}
		}
		OwnerPawn->SetActorHiddenInGame(true);
		OwnerPawn->SetActorTickEnabled(false); // Completely disable tick
		break;
	}
	
	// Re-enable tick if not culled
	if (NewLODLevel != EAILODLevel::Culled && !OwnerPawn->IsActorTickEnabled())
	{
		OwnerPawn->SetActorTickEnabled(true);
	}

	UE_LOG(LogTemp, Log, TEXT("%s new LOD is %d"), *OwnerPawn->GetName(), static_cast<int32>(NewLODLevel));
}

