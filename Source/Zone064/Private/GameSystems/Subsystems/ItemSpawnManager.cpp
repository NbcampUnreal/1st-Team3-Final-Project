#include "GameSystems/Subsystems/ItemSpawnManager.h"

#include "ActorComponent/Storage/ZNContainerComponent.h"
#include "Data/ZNItemData.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "GameSystems/Subsystems/GameFlowManager.h"
#include "Engine/DataTable.h"
#include "Item/ZNBaseStorage.h"
#include "EngineUtils.h"
#include "Algo/RandomShuffle.h"
#include "Item/ZNBasePickup.h"

void UItemSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CacheItemDataFromTable();
	BuildItemPools();
	InitFuelData();
}

void UItemSpawnManager::CacheItemDataFromTable()
{
	// GameInstance에서 ItemPickupDataTable 가져오기
	UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (!GameInstance) return;
	
	UDataTable* ItemPickupDataTable = GameInstance->ItemPickupDataTable;
	if (!ItemPickupDataTable) return;
	
	// DataTable에서 모든 아이템 데이터 로드
	TArray<FItemPickupDataRow*> AllRows;
	ItemPickupDataTable->GetAllRows<FItemPickupDataRow>(TEXT("ItemSpawnManager"), AllRows);
	
	CachedItemDataList.Empty();
	for (FItemPickupDataRow* Row : AllRows)
	{
		if (Row && Row->ItemData)
		{
			CachedItemDataList.Add(Row->ItemData);
		}
	}
}

void UItemSpawnManager::BuildItemPools()
{
	ItemPoolsByType.Empty();
	
	// 캐싱된 아이템들을 타입별로 분류
	for (UZNItemData* ItemData : CachedItemDataList)
	{
		if (ItemData)
		{
			EItemType ItemType = ItemData->ItemType;
			if (!ItemPoolsByType.Contains(ItemType))
			{
				ItemPoolsByType.Add(ItemType, TArray<UZNItemData*>());
			}
			ItemPoolsByType[ItemType].Add(ItemData);
		}
	}
}

void UItemSpawnManager::InitFuelData()
{
	if (UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetGameInstance()))
	{
		TotalFuelPerPhase = GameInstance->TotalFuelPerPhase;
		if (UZNItemData* FuelData = GameInstance->GetItemDataFromPickup("Fuel"))
		{
			FuelItemId = FuelData->GetPrimaryAssetId();
			FuelDurability = FuelData->Durability;
		}
	}
}

void UItemSpawnManager::SpawnFuelItems()
{
	if (!FuelItemId.IsValid()) return;
	TArray<UZNContainerComponent*> AllContainers = FindAllContainerComponents();
	Algo::RandomShuffle(AllContainers);

	int32 RemainingFuelAmount = TotalFuelPerPhase;
	for (UZNContainerComponent* ContainerComp : AllContainers)
	{
		if (RemainingFuelAmount == 0) break;
		ContainerComp->AddItem(FuelItemId, 1, FuelDurability);
		RemainingFuelAmount--;
	}
}

void UItemSpawnManager::GiveStarterWeapons()
{
	if (GetCurrentRepeatCount() != 1) return;
	
	AActor* CarActor = FindCarStorageActor();
	if (!CarActor) return;
	
	UZNBaseStorageComponent* MainStorage = FindMainStorageInCar(CarActor);
	if (!MainStorage) return;
	
	if (UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetGameInstance()))
	{
		if (UZNItemData* WeaponData = GameInstance->GetItemDataFromPickup("NailedPlank"))
		{
			FPrimaryAssetId WeaponId = WeaponData->GetPrimaryAssetId();
			int32 WeaponDurability = WeaponData->Durability;
			
			for (int32 i = 0; i < 4; ++i)
			{
				MainStorage->AddItem(WeaponId, 1, WeaponDurability);
			}
		}
	}
}

void UItemSpawnManager::SpawnItemAtLocation(FVector Location, FName RowName)
{
	 if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client) return;
	
	UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->DefaultPickupActorClass) return;
	
	// 지정된 위치에 픽업 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	AZNBasePickup* SpawnedPickup = GetWorld()->SpawnActor<AZNBasePickup>(
		GameInstance->DefaultPickupActorClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	// Pickup Item에 데이터 처리 
	if (SpawnedPickup)
	{
		UZNItemData* ItemData = GameInstance->GetItemDataFromPickup(RowName);
		if (ItemData)
		{
			int32 SpawnQuantity = ItemData->GetRandomSpawnQuantity();
			int32 ItemDurability = ItemData->Durability;
			
			// SetItemData로 완전한 아이템 정보 설정
			SpawnedPickup->SetItemData(RowName, SpawnQuantity, ItemDurability);
		}
	}
}

void UItemSpawnManager::GiveAllWeaponsToCarStorage()
{
	AActor* CarActor = FindCarStorageActor();
	if (!CarActor) return;
	
	UZNBaseStorageComponent* MainStorage = FindMainStorageInCar(CarActor);
	if (!MainStorage) return;
	
	// 캐싱된 아이템 데이터에서 무기 카테고리 아이템들만 필터링
	for (UZNItemData* ItemData : CachedItemDataList)
	{
		if (ItemData && ItemData->ItemType == EItemType::Weapon)
		{
			FPrimaryAssetId WeaponId = ItemData->GetPrimaryAssetId();
			
			MainStorage->AddItem(WeaponId, 1, 1000);
		}
	}
}

TArray<FItemSpawnResult> UItemSpawnManager::GenerateRandomItems(int32 ItemCount, const TArray<FItemTypeSpawnChance>& CustomSpawnChances)
{
	TArray<FItemSpawnResult> SpawnResults;
	
	if (CachedItemDataList.Num() == 0 || CustomSpawnChances.Num() == 0)
	{
		return SpawnResults;
	}
	
	int32 CurrentRepeatCount = GetCurrentRepeatCount();
	
	for (int32 i = 0; i < ItemCount; ++i)
	{
		if (UZNItemData* SelectedItem = SelectRandomItemWithCustomChances(CustomSpawnChances, CurrentRepeatCount))
		{
			int32 Quantity = SelectedItem->GetRandomSpawnQuantity();
			int32 Durability = SelectedItem->Durability;
			
			SpawnResults.Add(FItemSpawnResult(SelectedItem->GetPrimaryAssetId(), Quantity, Durability));
		}
	}
	
	return SpawnResults;
}

int32 UItemSpawnManager::GetCurrentRepeatCount() const
{
	UGameFlowManager* GameFlowManager = GetGameInstance()->GetSubsystem<UGameFlowManager>();
	if (!GameFlowManager) return 1;
	
	return GameFlowManager->GetCurRepeatCountCache();
}

UZNItemData* UItemSpawnManager::SelectRandomItemWithCustomChances(const TArray<FItemTypeSpawnChance>& SpawnChances, int32 CurrentRepeatCount)
{
	// 1단계: 가중치 기반 타입 선택
	EItemType SelectedType = SelectRandomItemTypeFromChances(SpawnChances);
	
	// 2단계: 해당 타입에서 RepeatCount 조건을 만족하는 아이템들 필터링
	if (ItemPoolsByType.Contains(SelectedType))
	{
		const TArray<UZNItemData*>& TypePool = ItemPoolsByType[SelectedType];
		TArray<UZNItemData*> ValidItems;
		TArray<float> ItemWeights;
		
		// RepeatCount 조건을 만족하는 아이템들만 수집
		for (UZNItemData* ItemData : TypePool)
		{
			if (ItemData && ItemData->CanSpawnAtRepeatCount(CurrentRepeatCount))
			{
				ValidItems.Add(ItemData);
				ItemWeights.Add(ItemData->SpawnWeight);
			}
		}
		
		// 가중치 기반 아이템 선택
		if (ValidItems.Num() > 0)
		{
			return SelectRandomItemByWeight(ValidItems, ItemWeights);
		}
	}
	
	return nullptr;
}

EItemType UItemSpawnManager::SelectRandomItemTypeFromChances(const TArray<FItemTypeSpawnChance>& SpawnChances)
{
	// 가용한 타입과 가중치 수집
	TArray<float> Weights;
	TArray<EItemType> Types;
	
	for (const FItemTypeSpawnChance& SpawnChance : SpawnChances)
	{
		// 해당 타입의 아이템이 실제로 존재하는지 확인
		if (SpawnChance.SpawnProbability > 0.0f && 
			ItemPoolsByType.Contains(SpawnChance.ItemType) && 
			ItemPoolsByType[SpawnChance.ItemType].Num() > 0)
		{
			Weights.Add(SpawnChance.SpawnProbability);
			Types.Add(SpawnChance.ItemType);
		}
	}
	
	if (Types.Num() == 0)
	{
		return EItemType::Crafting; 
	}
	
	// 가중치 합계 계산
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}
	
	if (TotalWeight <= 0.0f)
	{
		return Types[0]; 
	}
	
	// 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;
	
	for (int32 i = 0; i < Types.Num(); ++i)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return Types[i];
		}
	}
	
	return Types.Last(); 
}

UZNItemData* UItemSpawnManager::SelectRandomItemByWeight(const TArray<UZNItemData*>& Items, const TArray<float>& Weights)
{
	if (Items.Num() == 0 || Items.Num() != Weights.Num())
	{
		return nullptr;
	}
	
	// 가중치 합계 계산
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}
	
	if (TotalWeight <= 0.0f)
	{
		// 가중치가 모두 0이면 균등 확률로 선택
		int32 RandomIndex = FMath::RandRange(0, Items.Num() - 1);
		return Items[RandomIndex];
	}
	
	// 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;
	
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return Items[i];
		}
	}
	
	return Items.Last(); 
}

TArray<UZNContainerComponent*> UItemSpawnManager::FindAllContainerComponents() const
{
	TArray<UZNContainerComponent*> FoundContainers;

	if (!GetWorld()) return FoundContainers;
	
	for (TActorIterator<AZNBaseStorage> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (UZNContainerComponent* ContainerComp = Actor->FindComponentByClass<UZNContainerComponent>())
		{
			FoundContainers.Add(ContainerComp);
		}
	}
	return FoundContainers;
}

AActor* UItemSpawnManager::FindCarStorageActor() const
{
	if (!GetWorld()) return nullptr;
	
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor && Actor->ActorHasTag(FName("CarStorage")))
		{
			return Actor;
		}
	}
	
	return nullptr;
}

UZNBaseStorageComponent* UItemSpawnManager::FindMainStorageInCar(AActor* CarActor) const
{
	if (!CarActor) return nullptr;
	
	TArray<UZNBaseStorageComponent*> StorageComponents;
	CarActor->GetComponents<UZNBaseStorageComponent>(StorageComponents);
	
	for (UZNBaseStorageComponent* Storage : StorageComponents)
	{
		if (Storage)
		{
			FGameplayTagContainer AllowedTags = Storage->GetAllowedItemTags();
			
			// 조건 1: AllowedItemTags가 비어있음
			if (AllowedTags.IsEmpty())
			{
				return Storage;
			}
			
			// 조건 2: "Item" 태그만 있음
			if (AllowedTags.Num() == 1 && 
				AllowedTags.HasTag(FGameplayTag::RequestGameplayTag("Item")))
			{
				return Storage;
			}
		}
	}
	
	return nullptr;
}
