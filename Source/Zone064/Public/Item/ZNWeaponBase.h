#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZNWeaponBase.generated.h"

UCLASS()
class ZONE064_API AZNWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AZNWeaponBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
