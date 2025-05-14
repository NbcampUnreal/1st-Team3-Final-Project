#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZNInventoryComponent.generated.h"

class AZNInventoryTestBaseItem;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Items")
	TArray<AZNInventoryTestBaseItem*> Items;

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

protected:
	virtual void BeginPlay() override;
	
};
