#include "Character/Inventory/ZNInventoryComponent.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "UI/Inventory/ZNInventoryGridWidget.h"

UZNInventoryComponent::UZNInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UZNInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Success"));
}

void UZNInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAddedItem)
	{
		InventoryGridWidgetReference->Refresh();
		bAddedItem = false;
	}
}

bool UZNInventoryComponent::TryAddItem(AZNInventoryTestBaseItem* Item)
{
	if (Item)
	{
		for (int32 i = 0; i < Items.Num(); i++)
		{
			if (IsRoomAvailable(Item, i))
			{
				AddItemAt(Item, i);
				return true;
			}
		}

		return false;
	}

	return false;
}

bool UZNInventoryComponent::IsRoomAvailable(AZNInventoryTestBaseItem* Item, int32 TopLeftIndex)
{
	FIntPoint Dimensions = Item->GetDimensions();
	FIntPoint Tile = IndexToTile(TopLeftIndex);

	// Columns
	for (int32 x = Tile.X; x < Tile.X + Dimensions.X; x++)
	{
		// Rows
		for (int32 y = Tile.Y; y < Tile.Y + Dimensions.Y; y++)
		{
			if (IsTileValid(FIntPoint(x, y)))
			{
				int32 Index = TileToIndex(FIntPoint(x, y));

				if (IsIndexValid(Index))
				{
					if (GetItemAtIndex(Index))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

FIntPoint UZNInventoryComponent::IndexToTile(int32 Index)
{
	// 아이템 배열의 index를 좌표로 변환(== [Columns, Rows])
	return FIntPoint(Index % Columns, Index / Columns);
}

int32 UZNInventoryComponent::TileToIndex(FIntPoint Tile)
{
	return Tile.X + Tile.Y * Columns;
}

bool UZNInventoryComponent::IsTileValid(FIntPoint Tile)
{
	if (Tile.X >= 0 && Tile.Y >= 0 && Tile.X < Columns && Tile.Y < Rows)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UZNInventoryComponent::IsIndexValid(int32 Index)
{
	// IsValidIndex : TArray 내장 함수
	if (Items.IsValidIndex(Index))
	{
		return true;
	}
	else
	{
		return false;
	}
}

AZNInventoryTestBaseItem* UZNInventoryComponent::GetItemAtIndex(int32 Index)
{
	if (Items.IsValidIndex(Index))
	{
		return Items[Index];
	}
	else
	{
		return nullptr;
	}
}

void UZNInventoryComponent::AddItemAt(AZNInventoryTestBaseItem* Item, int32 TopLeftIndex)
{
	FIntPoint Dimensions = Item->GetDimensions();
	FIntPoint Tile = IndexToTile(TopLeftIndex);

	for (int32 x = Tile.X; x < Tile.X + Dimensions.X; x++)
	{
		for (int32 y = Tile.Y; y < Tile.Y + Dimensions.Y; y++)
		{
			Items[TileToIndex(FIntPoint(x, y))] = Item;
		}
	}

	bAddedItem = true;
}

TMap<AZNInventoryTestBaseItem*, FIntPoint> UZNInventoryComponent::GetAllItems()
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i])
		{
			if (!AllItems.Contains(Items[i]))
			{
				AllItems.Add(Items[i], IndexToTile(i));
			}
		}
	}

	return AllItems;
}

void UZNInventoryComponent::SetInventoryGridWidget(UZNInventoryGridWidget* GridWidget)
{
	InventoryGridWidgetReference = GridWidget;
}