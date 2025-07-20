#include "ActorComponent/Storage/ZNBaseStorageComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "Data/ZNItemData.h"
#include "ActorComponent/HealthComponent.h"
#include "ActorComponent/HungerComponent.h"
#include "Item/ZNBasePickup.h"
#include "GameInstance/ZNBaseGameInstance.h"

/*
* --- FZNInventoryList ---
*/

bool FZNInventoryList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FZNInventorySlotInfo, FZNInventoryList>(Items, DeltaParms, *this);
}

/*
* --- UZNBaseStorageComponent ---
*/

UZNBaseStorageComponent::UZNBaseStorageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxSlots = 20; 
}

void UZNBaseStorageComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryList.OwningComponent = this;
	
	CachePickupActorClass();

	if (GetOwner()->HasAuthority())
	{
		if (MaxSlots > 0)
		{
			InventoryList.Items.SetNum(MaxSlots);
			
			// 각 슬롯에 인덱스 할당
			for (int32 i = 0; i < MaxSlots; ++i)
			{
				InventoryList.Items[i].SlotIndex = i;
			}
			
			InventoryList.MarkArrayDirty();
		}
	}
}

void UZNBaseStorageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UZNBaseStorageComponent, InventoryList);
}

void UZNBaseStorageComponent::AddItem(const FPrimaryAssetId& ItemId, const int32& Quantity, const int32& Durability)
{
	if (GetOwner()->HasAuthority())
	{
		Server_AddItem_Implementation(ItemId, Quantity, Durability);
	}
	else
	{
		Server_AddItem(ItemId, Quantity, Durability);
	}
}

void UZNBaseStorageComponent::RemoveItem(const FPrimaryAssetId& ItemId, const int32& Quantity, const int32& Durability)
{
	if (GetOwner()->HasAuthority())
	{
		Server_RemoveItem_Implementation(ItemId, Quantity, Durability);
	}
	else
	{
		Server_RemoveItem(ItemId, Quantity, Durability);
	}
}

void UZNBaseStorageComponent::SortInventory()
{
	if (GetOwner()->HasAuthority())
	{
		Server_SortInventory_Implementation();
	}
	else
	{
		Server_SortInventory();
	}
}

void UZNBaseStorageComponent::UseConsumableItemFromSlot(APawn* PlayerPawn, int32 SlotIndex)
{
	if (GetOwner()->HasAuthority())
	{
		Server_UseConsumableItemFromSlot_Implementation(PlayerPawn, SlotIndex);
	}
	else
	{
		Server_UseConsumableItemFromSlot(PlayerPawn, SlotIndex);
	}
}

void UZNBaseStorageComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Quantity)
{
	if (GetOwner()->HasAuthority())
	{
		Server_RemoveItemFromSlot_Implementation(SlotIndex, Quantity);
	}
	else
	{
		Server_RemoveItemFromSlot(SlotIndex, Quantity);
	}
}


void UZNBaseStorageComponent::RequestTransferItemBetweenSlots(UZNBaseStorageComponent* OtherStorage, int32 SourceSlotIndex, int32 TargetSlotIndex)
{
	// 플레이어 Storage를 통한 범용 RPC 호출
	if (UZNBaseStorageComponent* PlayerStorage = GetPlayerStorageComponent())
	{
		PlayerStorage->Server_TransferBetweenAnyStorages(this, OtherStorage, SourceSlotIndex, TargetSlotIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find player storage component"));
	}
}

void UZNBaseStorageComponent::TransferItemBetweenSlots(
    UZNBaseStorageComponent* OtherStorage, int32 SourceSlotIndex, int32 TargetSlotIndex)
{
    // --- 1. 유효성 검사 ---
    if (!OtherStorage || !GetOwner()->HasAuthority()) return;
    if (!InventoryList.Items.IsValidIndex(SourceSlotIndex)) return;
    if (!OtherStorage->InventoryList.Items.IsValidIndex(TargetSlotIndex)) return;
	if (this == OtherStorage && SourceSlotIndex == TargetSlotIndex)
	{
		FZNInventorySlotInfo& Slot = InventoryList.Items[SourceSlotIndex];
		NotifyInventorySlotUpdated(Slot);
		return;
	}
	
    FZNInventorySlotInfo& SourceSlot = InventoryList.Items[SourceSlotIndex];
    FZNInventorySlotInfo& TargetSlot = OtherStorage->InventoryList.Items[TargetSlotIndex];

    // --- 2. 로직 분기 ---
    if (SourceSlot.ItemId == TargetSlot.ItemId)
    {
        // --- Case 1: 같은 아이템 (합치기) ---
        if (!SourceSlot.IsValid()) return; // 옮길 아이템이 없음

		// 합치기 전, 스택 가능 여부와 내구도 일치 여부 확인
		const int32 MaxStackSize = GetMaxStackSize(SourceSlot.ItemId);
		const bool bCanStack = MaxStackSize > 1;
		const bool bDurabilityMatches = SourceSlot.Durability == TargetSlot.Durability;

        if (bCanStack && bDurabilityMatches)
        {
			const int32 AmountToMove = FMath::Min(SourceSlot.Quantity, MaxStackSize - TargetSlot.Quantity);

			if (AmountToMove > 0)
			{
				TargetSlot.Quantity += AmountToMove;
				SourceSlot.Quantity -= AmountToMove;

				if (SourceSlot.Quantity <= 0)
				{
					ClearSlot(SourceSlot);
				}
				
				InventoryList.MarkItemDirty(SourceSlot);
				OtherStorage->InventoryList.MarkItemDirty(TargetSlot);
				NotifyInventorySlotUpdated(SourceSlot);
				OtherStorage->NotifyInventorySlotUpdated(TargetSlot);
			}
        }
    }
    else
    {
        // --- Case 2 & 3: 다른 아이템(교환) 또는 빈 슬롯(이동) ---

        // 각 아이템이 상대방 인벤토리에 들어갈 수 있는지 태그 검사
        const bool bCanSourceGoToTarget = !SourceSlot.IsValid() || OtherStorage->CanStoreItem(SourceSlot.ItemId);
        const bool bCanTargetGoToSource = !TargetSlot.IsValid() || CanStoreItem(TargetSlot.ItemId);

        if (bCanSourceGoToTarget && bCanTargetGoToSource)
        {
            // 필드별 교체 방식으로 변경 (ReplicationID 유지)
            FPrimaryAssetId TempItemId = SourceSlot.ItemId;
            int32 TempQuantity = SourceSlot.Quantity;
            int32 TempDurability = SourceSlot.Durability;

            // SourceSlot에 TargetSlot 데이터 복사
            SourceSlot.ItemId = TargetSlot.ItemId;
            SourceSlot.Quantity = TargetSlot.Quantity;
            SourceSlot.Durability = TargetSlot.Durability;
        	
            // TargetSlot에 원래 SourceSlot 데이터 복사
            TargetSlot.ItemId = TempItemId;
            TargetSlot.Quantity = TempQuantity;
            TargetSlot.Durability = TempDurability;
        	
            InventoryList.MarkItemDirty(SourceSlot);
            OtherStorage->InventoryList.MarkItemDirty(TargetSlot);
            NotifyInventorySlotUpdated(SourceSlot);
            OtherStorage->NotifyInventorySlotUpdated(TargetSlot);
        }
        else
        {
        	NotifyInventorySlotUpdated(SourceSlot);
        }
    }
}

UZNBaseStorageComponent* UZNBaseStorageComponent::GetPlayerStorageComponent() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				return PlayerPawn->FindComponentByClass<UZNBaseStorageComponent>();
			}
		}
	}
	return nullptr;
}

void UZNBaseStorageComponent::ClearSlot(FZNInventorySlotInfo& Slot)
{
	Slot.ItemId = FPrimaryAssetId();
	Slot.Quantity = 0;
	Slot.Durability = 0;
}

int32 UZNBaseStorageComponent::GetMaxStackSize(const FPrimaryAssetId& ItemId) const
{
	const UZNItemData* ItemData = GetItemData(ItemId);
	if (!ItemData) return -1;

	return ItemData->GetEffectiveStackSize();
}

bool UZNBaseStorageComponent::IsFull() const
{
	if (MaxSlots <= 0) return false;

	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (!Slot.IsValid())
		{
			return false;
		}
	}
	return true;
}

bool UZNBaseStorageComponent::HasItem(const FPrimaryAssetId& ItemId) const
{
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (Slot.ItemId == ItemId)
		{
			return true;
		}
	}
	return false;
}

int32 UZNBaseStorageComponent::GetItemCount(const FPrimaryAssetId& ItemId) const
{
	int32 TotalCount = 0;
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (Slot.ItemId == ItemId)
		{
			TotalCount += Slot.Quantity;
		}
	}
	return TotalCount;
}

bool UZNBaseStorageComponent::FindItem(const FPrimaryAssetId& ItemId, FZNInventorySlotInfo& OutSlotInfo) const
{
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (Slot.ItemId == ItemId)
		{
			OutSlotInfo = Slot;
			return true;
		}
	}
	return false;
}

bool UZNBaseStorageComponent::CanStoreItem(const FPrimaryAssetId& ItemId) const
{
	if (AllowedItemTags.IsEmpty())
	{
		return true;
	}

	const UZNItemData* ItemData = GetItemData(ItemId);
	if (ItemData && ItemData->ItemTags.HasAny(AllowedItemTags))
	{
		return true;
	}

	return false;
}

UZNItemData* UZNBaseStorageComponent::GetItemData(const FPrimaryAssetId& ItemId)
{
	if (!ItemId.IsValid()) return nullptr;

	UAssetManager& AssetManager = UAssetManager::Get();
	return AssetManager.GetPrimaryAssetObject<UZNItemData>(ItemId);
}

void UZNBaseStorageComponent::HandleInventoryUpdated(const FZNInventorySlotInfo& UpdatedSlotInfo)
{
	OnInventoryUpdated.Broadcast(UpdatedSlotInfo);
	UE_LOG(LogTemp, Warning, TEXT("OnInventoryUpdated Delegate Called"));
}

int32 UZNBaseStorageComponent::CheckAddableQuantity(const FPrimaryAssetId& ItemId, int32 Quantity) const
{
	if (!CanStoreItem(ItemId) || Quantity <= 0)
	{
		return 0;
	}

	const int32 MaxStackSize = GetMaxStackSize(ItemId);
	int32 SpaceAvailable = 0;

	// 1. 기존 스택에서 남은 공간 계산
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (SpaceAvailable >= Quantity)
		{
			return Quantity;
		}
		
		if (Slot.ItemId == ItemId && Slot.Quantity < MaxStackSize)
		{
			SpaceAvailable += (MaxStackSize - Slot.Quantity);
		}
	}
	
	// 2. 빈 슬롯에서 추가될 공간 계산
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (SpaceAvailable >= Quantity)
		{
			return Quantity;
		}
		
		if (!Slot.IsValid())
		{
			SpaceAvailable += MaxStackSize;
		}
	}

	return FMath::Min(Quantity, SpaceAvailable);
}

FZNInventorySlotInfo UZNBaseStorageComponent::GetSlotByIndex(int32 SlotIndex) const
{
	if (InventoryList.Items.IsValidIndex(SlotIndex))
	{
		return InventoryList.Items[SlotIndex];
	}
	
	// 유효하지 않으면 빈 슬롯 반환 (SlotIndex = -1)
	return FZNInventorySlotInfo();
}

/*
* --- Server RPCs Implementation ---
*/

bool UZNBaseStorageComponent::Server_AddItem_Validate(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability)
{
	return ItemId.IsValid() && Quantity > 0;
}

void UZNBaseStorageComponent::Server_AddItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability)
{
	if (!CanStoreItem(ItemId)) return;
	
	const int32 MaxStackSize = GetMaxStackSize(ItemId);
	int32 RemainingQuantity = Quantity;

	// 1. Stack on to existing items
	for (int32 i = 0; i < InventoryList.Items.Num() && RemainingQuantity > 0; ++i)
	{
		FZNInventorySlotInfo& Slot = InventoryList.Items[i];
		if (Slot.ItemId == ItemId && Slot.Quantity < MaxStackSize)
		{
			const int32 AmountToAdd = FMath::Min(RemainingQuantity, MaxStackSize - Slot.Quantity);
			Slot.Quantity += AmountToAdd;
			RemainingQuantity -= AmountToAdd;
			InventoryList.MarkItemDirty(Slot);
			NotifyInventorySlotUpdated(Slot);
		}
	}

	// 2. Add to new slots
	if (RemainingQuantity > 0)
	{
		for (int32 i = 0; i < InventoryList.Items.Num() && RemainingQuantity > 0; ++i)
		{
			FZNInventorySlotInfo& Slot = InventoryList.Items[i];
			if (!Slot.IsValid())
			{
				const int32 AmountToAdd = FMath::Min(RemainingQuantity, MaxStackSize);
				Slot.ItemId = ItemId;
				Slot.Quantity = AmountToAdd;
				Slot.Durability = Durability;
				Slot.SlotIndex = i;  // 슬롯 인덱스 설정
				RemainingQuantity -= AmountToAdd;
				InventoryList.MarkItemDirty(Slot);
				Multicast_NotifyInventorySlotUpdated(Slot);
			}
		}
	}
}

bool UZNBaseStorageComponent::Server_RemoveItem_Validate(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability)
{
	return ItemId.IsValid() && Quantity > 0;
}

void UZNBaseStorageComponent::Server_RemoveItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability)
{
	int32 RemainingQuantityToRemove = Quantity;

	for (int32 i = InventoryList.Items.Num() - 1; i >= 0 && RemainingQuantityToRemove > 0; --i)
	{
		FZNInventorySlotInfo& Slot = InventoryList.Items[i];
		if (Slot.ItemId == ItemId)
		{
			const int32 AmountToRemove = FMath::Min(RemainingQuantityToRemove, Slot.Quantity);
			Slot.Quantity -= AmountToRemove;
			RemainingQuantityToRemove -= AmountToRemove;

			if (Slot.Quantity <= 0)
			{
				ClearSlot(Slot);
			}
			InventoryList.MarkItemDirty(Slot);
			NotifyInventorySlotUpdated(Slot);
		}
	}
}

void UZNBaseStorageComponent::Server_RemoveItemFromSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
	if (!GetOwner()->HasAuthority() || !InventoryList.Items.IsValidIndex(SlotIndex) || Quantity <= 0) return;

	FZNInventorySlotInfo& Slot = InventoryList.Items[SlotIndex];
	if (!Slot.IsValid()) return;

	const int32 AmountToRemove = FMath::Min(Quantity, Slot.Quantity);
	Slot.Quantity -= AmountToRemove;

	if (Slot.Quantity <= 0)
	{
		ClearSlot(Slot); 
	}

	InventoryList.MarkItemDirty(Slot);
	NotifyInventorySlotUpdated(Slot);
}

void UZNBaseStorageComponent::Server_SortInventory_Implementation()
{
	if (!GetOwner()->HasAuthority()) return;

	// 1. 유효한 아이템만 새 배열로 복사
	TArray<FZNInventorySlotInfo> ValidItems;
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		if (Slot.IsValid())
		{
			ValidItems.Add(Slot);
		}
	}

	// 2. 새 배열을 정렬
	ValidItems.Sort([this](const FZNInventorySlotInfo& A, const FZNInventorySlotInfo& B)
	{
		const UZNItemData* ItemDataA = GetItemData(A.ItemId);
		const UZNItemData* ItemDataB = GetItemData(B.ItemId);

		if (!ItemDataA || !ItemDataB)
		{
			// 유효하지 않은 데이터는 뒤로 보냄
			return !ItemDataA;
		}

		// 1. 아이템 타입으로 1차 정렬 (오름차순)
		if (ItemDataA->ItemType != ItemDataB->ItemType)
		{
			return static_cast<uint8>(ItemDataA->ItemType) < static_cast<uint8>(ItemDataB->ItemType);
		}

		// 2. 아이템 ID로 2차 정렬 (이름순)
		if (A.ItemId != B.ItemId)
		{
			return ItemDataA->ItemName.ToString() < ItemDataB->ItemName.ToString();
		}

		// 3. 내구도로 3차 정렬 (내림차순 - 높은 내구도가 먼저)
		if (A.Durability != B.Durability)
		{
			return A.Durability > B.Durability;
		}
		
		// 4. 수량으로 4차 정렬 (내림차순)
		return A.Quantity > B.Quantity;
	});

	// 3. 원본 인벤토리 리스트를 정렬된 결과로 재구성
	InventoryList.Items.Empty(MaxSlots);
	InventoryList.Items.Append(ValidItems);
	InventoryList.Items.SetNum(MaxSlots); // 빈 슬롯 추가
	
	// 4. 모든 슬롯의 인덱스 재할당
	for (int32 i = 0; i < InventoryList.Items.Num(); ++i)
	{
		InventoryList.Items[i].SlotIndex = i;
	}

	// 5. 마무리
	InventoryList.MarkArrayDirty();
	
	// 6. 모든 슬롯 UI 업데이트 (정렬 후 전체 인벤토리 갱신)
	for (const FZNInventorySlotInfo& Slot : InventoryList.Items)
	{
		// 추후 WBP 정렬 기능에서 수동으로 전체 슬롯 Refresh 함수 호출하도록 수정 필요
		NotifyInventorySlotUpdated(Slot);
	}
}

void UZNBaseStorageComponent::Server_UseConsumableItemFromSlot_Implementation(APawn* PlayerPawn, int32 SlotIndex)
{
	if (!PlayerPawn) return;
	
	FZNInventorySlotInfo TempSlot = GetSlotByIndex(SlotIndex);
	if (!TempSlot.IsValid()) return;

	UZNItemData* ItemData = GetItemData(TempSlot.ItemId);
	if (!ItemData || ItemData->ItemType != EItemType::Consumable) return;

	switch (ItemData->ConsumableSubType)
	{
	case EConsumableSubType::Health:
		if (UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>())
		{
			HealthComp->AddHealth(ItemData->ConsumableEffectAmount);
		}
		break;

	case EConsumableSubType::Hunger:
		if (UHungerComponent* HungerComp = PlayerPawn->FindComponentByClass<UHungerComponent>())
		{
			HungerComp->IncreaseHunger(ItemData->ConsumableEffectAmount);
		}
		break;

	default:
		return;
	}

	RemoveItemFromSlot(SlotIndex, 1);
}

void UZNBaseStorageComponent::Server_TransferBetweenAnyStorages_Implementation(
    UZNBaseStorageComponent* SourceStorage, 
    UZNBaseStorageComponent* TargetStorage, 
    int32 SourceSlotIndex, 
    int32 TargetSlotIndex)
{
    if (!SourceStorage || !TargetStorage || !GetOwner()->HasAuthority()) return;
    
    // SourceStorage의 TransferItemBetweenSlots 함수를 직접 호출
    SourceStorage->TransferItemBetweenSlots(TargetStorage, SourceSlotIndex, TargetSlotIndex);
}

void UZNBaseStorageComponent::Client_NotifyInventorySlotUpdated_Implementation(const FZNInventorySlotInfo& SlotInfo)
{
	HandleInventoryUpdated(SlotInfo);
}

void UZNBaseStorageComponent::Multicast_NotifyInventorySlotUpdated_Implementation(const FZNInventorySlotInfo& SlotInfo)
{
	HandleInventoryUpdated(SlotInfo);
}

void UZNBaseStorageComponent::NotifyInventorySlotUpdated(const FZNInventorySlotInfo& SlotInfo)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		// 플레이어 소유 Storage → 소유자에게만 Client RPC
		if (APlayerController* PC = OwnerPawn->GetController<APlayerController>())
		{
			Client_NotifyInventorySlotUpdated(SlotInfo);
		}
	}
	else
	{
		// 공유 Storage (차량, 상자 등) → 모든 클라이언트에게 Multicast RPC
		Multicast_NotifyInventorySlotUpdated(SlotInfo);
	}
}

void UZNBaseStorageComponent::RequestDropItemToWorld(APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo)
{
	UZNBaseStorageComponent* PlayerStorage = GetPlayerStorageComponent();
	
	if (PlayerStorage)
	{
		// 플레이어 Storage의 RPC를 통해 처리 (권한 문제 해결)
		PlayerStorage->Server_DropItemFromAnyStorage(this, PlayerPawn, SlotInfo);
	}
}

void UZNBaseStorageComponent::Server_DropItemFromAnyStorage_Implementation(UZNBaseStorageComponent* SourceStorage, APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo)
{
	// 유효성 검사
	if (!SourceStorage || !PlayerPawn || !SlotInfo.IsValid() || !GetWorld()) return;
	
	// ItemData 로드하여 PickupRowName 확인
	UZNItemData* ItemData = GetItemData(SlotInfo.ItemId);
	if (!ItemData || ItemData->PickupRowName.IsNone()) return;
	
	// 드롭 위치 계산 (PlayerPawn 앞쪽 위쪽)
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector PlayerForward = PlayerPawn->GetActorForwardVector();
	FVector DropLocation = PlayerLocation + (PlayerForward * 70.0f) + FVector(0, 0, 100.0f);
	
	if (!CachedPickupActorClass)
	{
		CachePickupActorClass();
	}

	if (CachedPickupActorClass)
	{
		AZNBasePickup* PickupActor = GetWorld()->SpawnActor<AZNBasePickup>(CachedPickupActorClass);
		if (!PickupActor) return;
	
		// 아이템 데이터 설정
		PickupActor->SetActorLocation(DropLocation);
		PickupActor->SetItemData(ItemData->PickupRowName, SlotInfo.Quantity, SlotInfo.Durability);
	
		// SourceStorage에서 아이템 제거
		SourceStorage->RemoveItemFromSlot(SlotInfo.SlotIndex, SlotInfo.Quantity);	
	}
	else
	{
		NotifyInventorySlotUpdated(SlotInfo);
	}
}

void UZNBaseStorageComponent::CachePickupActorClass()
{
	if (CachedPickupActorClass) return; 
	
	if (UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetWorld()->GetGameInstance()))
	{
		CachedPickupActorClass = GameInstance->DefaultPickupActorClass;
	}
}


