#include "Data/ZNItemData.h"

UZNItemData::UZNItemData()
{
	MaxStackSize = 1;
	Durability = -1;
	
	// 스폰 시스템 기본값
	MinRepeatCount = 1;
	SpawnWeight = 1.0f;
	SpawnQuantityRange = FIntPoint(1, 1);
}

FPrimaryAssetId UZNItemData::GetItemPrimaryAssetId() const
{
	return GetPrimaryAssetId();
}

bool UZNItemData::IsStackable() const
{
	return MaxStackSize > 1;
}

int32 UZNItemData::GetEffectiveStackSize() const
{
	return MaxStackSize > 0 ? MaxStackSize : 1;
}

bool UZNItemData::CanSpawnAtRepeatCount(int32 CurrentRepeatCount) const
{
	return CurrentRepeatCount >= MinRepeatCount;
}

int32 UZNItemData::GetRandomSpawnQuantity() const
{
	// Stackable 아이템이고 범위가 설정되어 있으면 랜덤 수량 반환
	if (IsStackable() && SpawnQuantityRange.Y > SpawnQuantityRange.X)
	{
		return FMath::RandRange(SpawnQuantityRange.X, SpawnQuantityRange.Y);
	}
	
	// Stackable 아이템이지만 범위가 같으면 해당 값 반환
	if (IsStackable() && SpawnQuantityRange.X > 0)
	{
		return SpawnQuantityRange.X;
	}
	
	// 기본값: 1개
	return 1;
}


