#include "ActorComponent/Storage/ZNContainerComponent.h"
#include "GameSystems/Subsystems/ItemSpawnManager.h"

UZNContainerComponent::UZNContainerComponent()
{
	// 기본 설정
	MaxSlots = 20;
	bCanSpawnFuelItem = true;
	bUseDefaultSpawnChances = true;
	ItemCountRange = FIntPoint(3, 6);  // X=Min, Y=Max
}

void UZNContainerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// if (GetOwner()->HasAuthority())
	// {
	// 	// 기본 확률 초기화
	// 	InitializeDefaultSpawnChances();
	// 	
	// 	// ItemSpawnManager에서 아이템 생성
	// 	if (UItemSpawnManager* SpawnManager = GetOwner()->GetGameInstance()->GetSubsystem<UItemSpawnManager>())
	// 	{
	// 		int32 ItemCount = FMath::RandRange(
	// 			ItemCountRange.X,  // Min 값
	// 			ItemCountRange.Y   // Max 값
	// 		);
	// 		
	// 		TArray<FItemSpawnResult> SpawnResults = SpawnManager->GenerateRandomItems(
	// 			ItemCount, 
	// 			ItemTypeSpawnChances
	// 		);
	// 		
	// 		// 생성된 아이템들을 Storage에 추가
	// 		for (const FItemSpawnResult& Result : SpawnResults)
	// 		{
	// 			Server_AddItem_Implementation(Result.ItemId, Result.Quantity, Result.Durability);
	// 		}
	// 	}
	// }
}

void UZNContainerComponent::InitializeDefaultSpawnChances()
{
	if (bUseDefaultSpawnChances && ItemTypeSpawnChances.Num() == 0)
	{
		ItemTypeSpawnChances.Add(FItemTypeSpawnChance(EItemType::Weapon, 1.0f));
		ItemTypeSpawnChances.Add(FItemTypeSpawnChance(EItemType::Armor, 2.0f));
		ItemTypeSpawnChances.Add(FItemTypeSpawnChance(EItemType::RepairKit, 2.0f));
		ItemTypeSpawnChances.Add(FItemTypeSpawnChance(EItemType::Consumable, 15.0f));
		ItemTypeSpawnChances.Add(FItemTypeSpawnChance(EItemType::Crafting, 80.0f));
	}
}
