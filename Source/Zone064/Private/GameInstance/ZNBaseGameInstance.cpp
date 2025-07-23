#include "GameInstance/ZNBaseGameInstance.h"

const FItemPickupDataRow* UZNBaseGameInstance::GetItemPickupDataRow(const FName& RowName) const
{
	if (!ItemPickupDataTable) return nullptr;

	const FItemPickupDataRow* Row = ItemPickupDataTable->FindRow<FItemPickupDataRow>(RowName, TEXT("GetItemPickupDataRow"));
	if (!Row) return nullptr;

	return Row;
}

UZNItemData* UZNBaseGameInstance::GetItemDataFromPickup(const FName& RowName) const
{
	if (!ItemPickupDataTable) return nullptr;

	const FItemPickupDataRow* Row = GetItemPickupDataRow(RowName);
	return Row? Row->ItemData : nullptr;
}

