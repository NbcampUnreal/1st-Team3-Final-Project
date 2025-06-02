// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/DefenseComponent.h"

// Sets default values for this component's properties
UDefenseComponent::UDefenseComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDefenseComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentDefense = 0.f; // 기본값
	// ...
	
}


void UDefenseComponent::AddDefense(float Amount)
{
	CurrentDefense += Amount;
}

void UDefenseComponent::RemoveDefense(float Amount)
{
	CurrentDefense = FMath::Max(0.f, CurrentDefense - Amount);
}

float UDefenseComponent::ModifyIncomingDamage(float RawDamage) const
{
	return FMath::Max(0.f, RawDamage - CurrentDefense);
}
