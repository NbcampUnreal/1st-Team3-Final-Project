#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZNInventoryComponent.generated.h"

class AZNInventoryTestBaseItem;
class UZNInventoryGridWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UZNInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Inventory Columns")
	int32 Columns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Inventory Rows")
	int32 Rows;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Inventory TileSize")
	float TileSize;

	// 실제 인벤토리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Items")
	TArray<AZNInventoryTestBaseItem*> Items;

	bool bAddedItem = false;
	TObjectPtr<UZNInventoryGridWidget> InventoryGridWidgetReference;

protected:
	// 아이템과 저장 위치 저장
	TMap<AZNInventoryTestBaseItem*, FIntPoint> AllItems;

public:	
	UZNInventoryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool TryAddItem(AZNInventoryTestBaseItem* Item);
	// TopLeftIndex : 아이템이 인벤토리에 들어갔을 때의 왼쪽 상단 Tile의 index
	bool IsRoomAvailable(AZNInventoryTestBaseItem* Item, int32 TopLeftIndex);

	FIntPoint IndexToTile(int32 Index);
	int32     TileToIndex(FIntPoint Tile);
	bool IsTileValid(FIntPoint Tile);
	bool IsIndexValid(int32 Index);
	AZNInventoryTestBaseItem* GetItemAtIndex(int32 Index);

	void AddItemAt(AZNInventoryTestBaseItem* Item, int32 TopLeftIndex);

	TMap<AZNInventoryTestBaseItem*, FIntPoint> GetAllItems();

	void SetInventoryGridWidget(UZNInventoryGridWidget* GridWidget);

	void RemoveItem(AZNInventoryTestBaseItem* ItemToRemove);

	void RefreshAllItems();

protected:
	virtual void BeginPlay() override;
	
};
