#include "Character/Inventory/ZNInventoryComponent.h"

UZNInventoryComponent::UZNInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UZNInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Success"));
}

void UZNInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

