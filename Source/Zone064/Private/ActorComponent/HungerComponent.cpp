// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/HungerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHungerComponent::UHungerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	IgnoreMaps = {
		TEXT("LobbyMap"),
		TEXT("CampsiteLevel")
	};

	// ...
}


// Called when the game starts
void UHungerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority() && bAutoDecrease)
	{
		GetWorld()->GetTimerManager().SetTimer(
			HungerTimerHandle,
			this,
			&UHungerComponent::TickHunger,
			1.0f,
			true
		);
	}	

	CachedMapName = UGameplayStatics::GetCurrentLevelName(this, true);
}


void UHungerComponent::TickHunger()
{
	DecreaseHunger(HungerDecayRate);
}

void UHungerComponent::DecreaseHunger(float Amount)
{
	if (IsCurrentMapIgnored()) return;

	Hunger = FMath::Clamp(Hunger - Amount, 0.f, MaxHunger);
}

void UHungerComponent::IncreaseHunger(float Amount)
{
	Hunger = FMath::Clamp(Hunger + Amount, 0.f, MaxHunger);
}

float UHungerComponent::GetHungerPercent() const
{
	if (MaxHunger <= 0.f)
		return 0.f;

	return Hunger / MaxHunger;
}

void UHungerComponent::TriggerTickHungerTimer(bool IsDead)
{
	if (GetOwner()->HasAuthority()) 
	{
		if (IsDead)
		{
			if (GetWorld()->GetTimerManager().IsTimerActive(HungerTimerHandle))
			{
				GetWorld()->GetTimerManager().ClearTimer(HungerTimerHandle);
			}
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(
				HungerTimerHandle,
				this,
				&UHungerComponent::TickHunger,
				1.0f,
				true
			);
		}
	}
}

bool UHungerComponent::IsCurrentMapIgnored() const
{
	return IgnoreMaps.Contains(CachedMapName);
}

void UHungerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHungerComponent, Hunger);
	DOREPLIFETIME(UHungerComponent, MaxHunger);
}

void UHungerComponent::OnRep_Hunger()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_Hunger called. Hunger = %f"), Hunger);

}
