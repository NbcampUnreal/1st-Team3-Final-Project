#include "ActorComponent/Storage/ZNBaseStorageComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "Data/ZNItemData.h"

/*
* --- FZNInventorySlotInfo ---
*/

void FZNInventorySlotInfo::PreReplicatedRemove(const struct FZNInventoryList& InArraySerializer)
{
	// FFastArraySerializer 필수 함수, 빈 구현 유지
}

void FZNInventorySlotInfo::PostReplicatedAdd(const struct FZNInventoryList& InArraySerializer)
{
	// FFastArraySerializer 필수 함수, 빈 구현 유지
}

void FZNInventorySlotInfo::PostReplicatedChange(const struct FZNInventoryList& InArraySerializer)
{
	if (InArraySerializer.OwningComponent)
	{
		InArraySerializer.OwningComponent->HandleInventoryUpdated();
	}
}

/*
* --- FZNInventoryList ---
*/

void FZNInventoryList::PostReplicatedChange(const TArray<int32, TInlineAllocator<8>>& ChangedIndices, int32 FinalSize)
{
	// FFastArraySerializer 필수 함수, 빈 구현 유지
}

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

	if (GetOwner()->HasAuthority())
	{
		if (MaxSlots > 0)
		{
			InventoryList.Items.SetNum(MaxSlots);
			InventoryList.MarkArrayDirty();
		}
	}
}

void UZNBaseStorageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UZNBaseStorageComponent, InventoryList);
}

void UZNBaseStorageComponent::AddItem(FPrimaryAssetId ItemId, int32 Quantity)
{
	Server_AddItem(ItemId, Quantity);
}

void UZNBaseStorageComponent::RemoveItem(FPrimaryAssetId ItemId, int32 Quantity)
{
	Server_RemoveItem(ItemId, Quantity);
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

bool UZNBaseStorageComponent::HasItem(FPrimaryAssetId ItemId) const
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

int32 UZNBaseStorageComponent::GetItemCount(FPrimaryAssetId ItemId) const
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

bool UZNBaseStorageComponent::FindItem(FPrimaryAssetId ItemId, FZNInventorySlotInfo& OutSlotInfo) const
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

bool UZNBaseStorageComponent::CanStoreItem(FPrimaryAssetId ItemId) const
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

UZNItemData* UZNBaseStorageComponent::GetItemData(FPrimaryAssetId ItemId)
{
	if (!ItemId.IsValid()) return nullptr;

	UAssetManager& AssetManager = UAssetManager::Get();
	return AssetManager.GetPrimaryAssetObject<UZNItemData>(ItemId);
}

void UZNBaseStorageComponent::HandleInventoryUpdated()
{
	OnInventoryUpdated.Broadcast();
}

/*
* --- Server RPCs Implementation ---
*/

bool UZNBaseStorageComponent::Server_AddItem_Validate(FPrimaryAssetId ItemId, int32 Quantity)
{
	return ItemId.IsValid() && Quantity > 0;
}

void UZNBaseStorageComponent::Server_AddItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity)
{
	if (!CanStoreItem(ItemId)) return;

	const UZNItemData* ItemData = GetItemData(ItemId);
	if (!ItemData) return;

	int32 RemainingQuantity = Quantity;
	const int32 MaxStackSize = ItemData->MaxStackSize > 0 ? ItemData->MaxStackSize : 1;

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
				RemainingQuantity -= AmountToAdd;
				InventoryList.MarkItemDirty(Slot);
			}
		}
	}
	
	HandleInventoryUpdated();
}

bool UZNBaseStorageComponent::Server_RemoveItem_Validate(FPrimaryAssetId ItemId, int32 Quantity)
{
	return ItemId.IsValid() && Quantity > 0;
}

void UZNBaseStorageComponent::Server_RemoveItem_Implementation(FPrimaryAssetId ItemId, int32 Quantity)
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
				Slot.ItemId = FPrimaryAssetId();
				Slot.Quantity = 0;
			}
			InventoryList.MarkItemDirty(Slot);
		}
	}
	
	HandleInventoryUpdated();
}