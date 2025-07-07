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
	FZNInventorySlotInfo() : Quantity(0) {}
	FZNInventorySlotInfo(FPrimaryAssetId InId, int32 InQuantity) : ItemId(InId), Quantity(InQuantity) {}

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FPrimaryAssetId ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Quantity;

	bool IsValid() const { return ItemId.IsValid() && Quantity > 0; }

	void PreReplicatedRemove(const struct FZNInventoryList& InArraySerializer);
	void PostReplicatedAdd(const struct FZNInventoryList& InArraySerializer);
	void PostReplicatedChange(const struct FZNInventoryList& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FZNInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FZNInventorySlotInfo> Items;

	void PostReplicatedChange(const TArray<int32, TInlineAllocator<8>>& ChangedIndices, int32 FinalSize);
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

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

	// 아이템을 추가. (클라이언트/서버 양용)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FPrimaryAssetId ItemId, int32 Quantity);

	// 아이템을 제거. (클라이언트/서버 양용)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(FPrimaryAssetId ItemId, int32 Quantity);

	// 인벤토리의 모든 아이템 슬롯 정보 반환. (빈 슬롯 포함)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FZNInventorySlotInfo>& GetInventoryItems() const { return InventoryList.Items; }

	// PrimaryAssetId로부터 아이템의 PDA 반환.
	UFUNCTION(BlueprintPure, Category = "Inventory", meta = (DisplayName = "GetItemDataById"))
	static UZNItemData* GetItemData(FPrimaryAssetId ItemId);

	// 인벤토리 변경이 감지되었을 때 호출 (UI 업데이트용)
	void HandleInventoryUpdated();


	/*
	* --- 유틸리티 함수 ---
	*/

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxSlots() const { return MaxSlots; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FPrimaryAssetId ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FPrimaryAssetId ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindItem(FPrimaryAssetId ItemId, FZNInventorySlotInfo& OutSlotInfo) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanStoreItem(FPrimaryAssetId ItemId) const;

protected:
	UPROPERTY(Replicated)
	FZNInventoryList InventoryList;

	/*
	* --- Server RPC ---
	*/

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AddItem(FPrimaryAssetId ItemId, int32 Quantity);
	bool Server_AddItem_Validate(FPrimaryAssetId ItemId, int32 Quantity);
	void Server_AddItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RemoveItem(FPrimaryAssetId ItemId, int32 Quantity);
	bool Server_RemoveItem_Validate(FPrimaryAssetId ItemId, int32 Quantity);
	void Server_RemoveItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity);
};