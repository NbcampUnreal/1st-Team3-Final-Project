#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth, BlueprintReadWrite, Category = "Health")
	float CurrentHealth = 100.0f;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_CurrentHealth();

    void AddHealth(const float& Amount);
};
