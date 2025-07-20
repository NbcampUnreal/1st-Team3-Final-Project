#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/PrimaryAssetId.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "ZNBaseStorageComponent.generated.h"

class UZNItemData;

USTRUCT(BlueprintType)
struct FZNInventorySlotInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FZNInventorySlotInfo() : SlotIndex(-1), Quantity(0), Durability(0) {}
	FZNInventorySlotInfo(int32 InSlotIndex, FPrimaryAssetId InId, int32 InQuantity, int32 InDurability)
	: SlotIndex(InSlotIndex), ItemId(InId), Quantity(InQuantity), Durability(InDurability) {}

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SlotIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FPrimaryAssetId ItemId;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Quantity;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Durability;
	
	bool IsValid() const { return ItemId.IsValid() && Quantity > 0; }
};

USTRUCT(BlueprintType)
struct FZNInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FZNInventorySlotInfo> Items;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	UPROPERTY()
	UZNBaseStorageComponent* OwningComponent = nullptr;
};

template<>
struct TStructOpsTypeTraits<FZNInventoryList> : public TStructOpsTypeTraitsBase2<FZNInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const FZNInventorySlotInfo&, UpdatedSlotInfo);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UZNBaseStorageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UZNBaseStorageComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0"))
	int32 MaxSlots;

	// 이 보관함에 허용된 아이템 태그. 비어있으면 모든 아이템 허용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGameplayTagContainer AllowedItemTags;

public:	
	/*
	* --- Storage 기본 기능 ---
	*/
	
	// 아이템 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(const FPrimaryAssetId& ItemId, const int32& Quantity, const int32& Durability);

	// 아이템 제거 
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(const FPrimaryAssetId& ItemId, const int32& Quantity, const int32& Durability);

	// 지정된 슬롯에서 아이템 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemFromSlot(int32 SlotIndex, int32 Quantity);

	// 인벤토리 정렬(Type-Quantity-Name 순서 정렬)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory();

	// 소비 아이템 사용
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseConsumableItemFromSlot(APawn* PlayerPawn, int32 SlotIndex);

	// 다른 보관함의 특정 슬롯과 현재 보관함의 특정 슬롯 간에 아이템을 이동/교환 (래퍼 함수 - 범용 RPC 호출)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestTransferItemBetweenSlots(UZNBaseStorageComponent* OtherStorage, int32 SourceSlotIndex, int32 TargetSlotIndex);
	
	// 내부 로직: 특정 슬롯 간 아이템 이동/교환 (서버 RPC에서 직접 호출)
	void TransferItemBetweenSlots(UZNBaseStorageComponent* OtherStorage, int32 SourceSlotIndex, int32 TargetSlotIndex);

	// 아이템을 월드에 드롭 (래퍼 함수 - 범용 RPC 호출)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestDropItemToWorld(APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo);
	
	// 내부 로직: 아이템 월드 드롭 (서버 RPC에서 직접 호출)
	void DropItemToWorld(APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo);

	/*
	* --- 유틸리티 함수 ---
	*/

	// 슬롯 초기화 (ReplicationID 보존하면서 데이터만 초기화)
	void ClearSlot(FZNInventorySlotInfo& Slot);
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxSlots() const { return MaxSlots; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxStackSize(const FPrimaryAssetId& ItemId) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(const FPrimaryAssetId& ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(const FPrimaryAssetId& ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindItem(const FPrimaryAssetId& ItemId, FZNInventorySlotInfo& OutSlotInfo) const;

	// GameplayTag로 아이템을 저장할 수 있는지 확인
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanStoreItem(const FPrimaryAssetId& ItemId) const;

	// 지정된 아이템과 수량을 추가할 수 있는지 확인하고, 실제 추가 가능한 수량을 반환
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 CheckAddableQuantity(const FPrimaryAssetId& ItemId, int32 Quantity) const;

	// 인덱스로 슬롯 정보 직접 반환
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FZNInventorySlotInfo GetSlotByIndex(int32 SlotIndex) const;

	// 인벤토리의 모든 아이템 슬롯 정보 반환 (빈 슬롯 포함)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FZNInventorySlotInfo>& GetInventoryItems() const { return InventoryList.Items; }

	// PrimaryAssetId로부터 아이템의 PDA 반환
	UFUNCTION(BlueprintPure, Category = "Inventory", meta = (DisplayName = "Get Item Data By Id"))
	static UZNItemData* GetItemData(const FPrimaryAssetId& ItemId);

	// 통합 UI 업데이트 함수 (소유권 기반 자동 분기)
	void NotifyInventorySlotUpdated(const FZNInventorySlotInfo& SlotInfo);
	
	// 인벤토리 변경이 감지되었을 때 호출 (UI 업데이트용)
	void HandleInventoryUpdated(const FZNInventorySlotInfo& UpdatedSlotInfo);

	// 플레이어 Storage 컴포넌트 찾기 (범용 RPC 호출용)
	UZNBaseStorageComponent* GetPlayerStorageComponent() const;

protected:
	UPROPERTY(Replicated)
	FZNInventoryList InventoryList;

	// GameInstance에서 캐싱된 픽업 액터 클래스 (성능 최적화)
	UPROPERTY()
	TSubclassOf<class AZNBasePickup> CachedPickupActorClass;

	/*
	* --- Server RPC ---
	*/

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_AddItem(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);
	bool Server_AddItem_Validate(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);
	void Server_AddItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_RemoveItem(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);
	bool Server_RemoveItem_Validate(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);
	void Server_RemoveItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 Durability);

	// 지정된 슬롯에서 아이템을 제거
	UFUNCTION(Server, Reliable, Category = "Inventory")
	void Server_RemoveItemFromSlot(int32 SlotIndex, int32 Quantity);
	void Server_RemoveItemFromSlot_Implementation(int32 SlotIndex, int32 Quantity);

	// UI 전용: 모든 Storage 간 아이템 이동/교환 (플레이어 Storage에서만 호출)
	UFUNCTION(Server, Reliable, Category = "Inventory")
	void Server_TransferBetweenAnyStorages(UZNBaseStorageComponent* SourceStorage, UZNBaseStorageComponent* TargetStorage, int32 SourceSlotIndex, int32 TargetSlotIndex);
	void Server_TransferBetweenAnyStorages_Implementation(UZNBaseStorageComponent* SourceStorage, UZNBaseStorageComponent* TargetStorage, int32 SourceSlotIndex, int32 TargetSlotIndex);
	
	// 인벤토리 정렬
	UFUNCTION(Server, Reliable, Category = "Inventory")
	void Server_SortInventory();
	void Server_SortInventory_Implementation();

	// 소비 아이템 사용
	UFUNCTION(Server, Reliable, Category = "Inventory")
	void Server_UseConsumableItemFromSlot(APawn* PlayerPawn, int32 SlotIndex);
	void Server_UseConsumableItemFromSlot_Implementation(APawn* PlayerPawn, int32 SlotIndex);

	// UI 전용: 모든 Storage에서 아이템 월드 드롭 (플레이어 Storage에서만 호출)
	UFUNCTION(Server, Reliable, Category = "Inventory")
	void Server_DropItemFromAnyStorage(UZNBaseStorageComponent* SourceStorage, APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo);
	void Server_DropItemFromAnyStorage_Implementation(UZNBaseStorageComponent* SourceStorage, APawn* PlayerPawn, const FZNInventorySlotInfo& SlotInfo);

	/*
	* --- Client RPC ---
	*/

	// UI 업데이트를 위한 Client RPC (플레이어 소유 Storage용)
	UFUNCTION(Client, Reliable, Category = "Inventory")
	void Client_NotifyInventorySlotUpdated(const FZNInventorySlotInfo& SlotInfo);
	void Client_NotifyInventorySlotUpdated_Implementation(const FZNInventorySlotInfo& SlotInfo);

	/*
	* --- Multicast RPC ---
	*/
	
	// UI 업데이트를 위한 Multicast RPC (공유 Storage용)
	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void Multicast_NotifyInventorySlotUpdated(const FZNInventorySlotInfo& SlotInfo);
	void Multicast_NotifyInventorySlotUpdated_Implementation(const FZNInventorySlotInfo& SlotInfo);

private:
	// 픽업 액터 클래스 캐싱 (실패 시 재시도 가능)
	void CachePickupActorClass();
};
