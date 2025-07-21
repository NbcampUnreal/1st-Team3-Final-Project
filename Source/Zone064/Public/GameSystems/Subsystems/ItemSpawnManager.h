#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"
#include "Data/ZNItemData.h"
#include "ItemSpawnManager.generated.h"

class UZNContainerComponent;
class UZNBaseStorageComponent;

USTRUCT(BlueprintType)
struct FItemTypeSpawnChance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EItemType ItemType = EItemType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float SpawnProbability = 0.0f;
	
	FItemTypeSpawnChance()
		: ItemType(EItemType::None), SpawnProbability(0.0f)
	{
	}
	
	FItemTypeSpawnChance(EItemType Type, float Probability)
		: ItemType(Type), SpawnProbability(Probability)
	{
	}
};

USTRUCT(BlueprintType)
struct FItemSpawnResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item Spawn")
	FPrimaryAssetId ItemId;
	
	UPROPERTY(BlueprintReadWrite, Category = "Item Spawn")
	int32 Quantity = 1;
	
	UPROPERTY(BlueprintReadWrite, Category = "Item Spawn")
	int32 Durability = -1;

	FItemSpawnResult()
		: Quantity(1), Durability(-1)
	{
	}

	FItemSpawnResult(const FPrimaryAssetId& InItemId, int32 InQuantity, int32 InDurability)
		: ItemId(InItemId), Quantity(InQuantity), Durability(InDurability)
	{
	}
};

UCLASS()
class ZONE064_API UItemSpawnManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 월드의 Container Actor에 연료 스폰
	UFUNCTION(BlueprintCallable, Category = "Fuel Spawn")
	void SpawnFuelItems();
	
	// 시작 무기 지급 (1페이즈용)
	UFUNCTION(BlueprintCallable, Category = "Starter Weapon")
	void GiveStarterWeapons();
	
	// Container Actor에 아이템 스폰
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	TArray<FItemSpawnResult> GenerateRandomItems(int32 ItemCount, const TArray<FItemTypeSpawnChance>& CustomSpawnChances);
	
	// 특정 위치에 연료 아이템 스폰
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	void SpawnItemAtLocation(FVector Location, FName RowName = "Fuel");
	
	// 개발자 테스트용 - 모든 무기를 CarStorage에 지급
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void GiveAllWeaponsToCarStorage();

private:
	// 캐싱된 아이템 데이터
	UPROPERTY()
	TArray<UZNItemData*> CachedItemDataList;
	
	// 타입별로 분류된 아이템 풀 리스트
	TMap<EItemType, TArray<UZNItemData*>> ItemPoolsByType;

	// 연료 스폰 정보
	FPrimaryAssetId FuelItemId;
	int32 TotalFuelPerPhase;
	int32 FuelDurability;
	
	/*
	*	--- 초기화 로직 ---
	*/
	
	void CacheItemDataFromTable();
	void BuildItemPools();
	void InitFuelData();
	
	/*
	*	--- 아이템 스폰 확률 계산 로직 ---
	*/
	
	int32 GetCurrentRepeatCount() const;
	UZNItemData* SelectRandomItemWithCustomChances(const TArray<FItemTypeSpawnChance>& SpawnChances, int32 CurrentRepeatCount);
	EItemType SelectRandomItemTypeFromChances(const TArray<FItemTypeSpawnChance>& SpawnChances);
	UZNItemData* SelectRandomItemByWeight(const TArray<UZNItemData*>& Items, const TArray<float>& Weights);

	/*
	*	--- 유틸리티 ---
	*/
	
	TArray<UZNContainerComponent*> FindAllContainerComponents() const;
	AActor* FindCarStorageActor() const;
	UZNBaseStorageComponent* FindMainStorageInCar(AActor* CarActor) const;
	
};
