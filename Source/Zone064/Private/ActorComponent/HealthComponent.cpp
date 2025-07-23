#include "ActorComponent/HealthComponent.h"
#include "Net/UnrealNetwork.h"

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::AddHealth(const float& Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, 100.0f);
}
