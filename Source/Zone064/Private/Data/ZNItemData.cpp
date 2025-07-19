#include "Data/ZNItemData.h"

UZNItemData::UZNItemData()
{
	MaxStackSize = 1;
	Durability = -1;
}

bool UZNItemData::IsStackable() const
{
	return MaxStackSize > 1;
}

int32 UZNItemData::GetEffectiveStackSize() const
{
	return MaxStackSize > 0 ? MaxStackSize : 1;
}

