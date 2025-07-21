#pragma once

#include "CoreMinimal.h"
#include "ActorComponent/Storage/ZNBaseStorageComponent.h"
#include "GameSystems/Subsystems/ItemSpawnManager.h"
#include "ZNContainerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZONE064_API UZNContainerComponent : public UZNBaseStorageComponent
{
	GENERATED_BODY()

public:
	UZNContainerComponent();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container Spawn")
	TArray<FItemTypeSpawnChance> ItemTypeSpawnChances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container Spawn")
	FIntPoint ItemCountRange = FIntPoint(3, 6);  // 3~6개 아이템 (X=Min, Y=Max)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container Spawn")
	bool bUseDefaultSpawnChances = true;  // 기본 확률 사용 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container Spawn")
	bool bCanSpawnFuelItem = true;  // 연료 아이템 스폰 가능 여부

private:
	// 기본 확률 초기화
	void InitializeDefaultSpawnChances();
};
