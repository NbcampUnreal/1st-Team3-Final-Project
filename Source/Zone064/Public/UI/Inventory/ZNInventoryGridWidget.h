#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Inventory/ZNInventoryDataStruct.h"
#include "ZNInventoryGridWidget.generated.h"

class UCanvasPanel;
class UBorder;
struct FLines;
class UZNInventoryComponent;
class AZNInventoryTestCharacter;
class AZNInventoryTestBaseItem;

UCLASS()
class ZONE064_API UZNInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	bool bDropped;

	TObjectPtr<AZNInventoryTestBaseItem> DroppedItem;

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> Canvas;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBorder> GridBorder;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> GridCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Columns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Rows;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float TileSize;

	// ItemWidget을 붙일 PanelSlot
	TObjectPtr<UPanelSlot> PanelSlot;

	FLines* LineStructData;

	TArray<float> StartX;
	TArray<float> StartY;
	TArray<float> EndX;
	TArray<float> EndY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<AZNInventoryTestCharacter> CharacterReference;
	TObjectPtr<UZNInventoryComponent> InventoryComponent;

	FIntPoint DraggedItemTopLeftTile;
	FMousePositionInTile MousePositionInTile;

	TObjectPtr<AZNInventoryTestBaseItem> DraggedItem;
	TObjectPtr<UDragDropOperation> StoredDragOperation;
	// 이게 필요한가?
	// - 리팩토링을 잘 하면, 필요 없을 수도
	// - 현재는 이게 없으면 DragEnter, DragLeave에서 StoredDragOperation의 Payload를 건드려야하니 좀 그렇긴함
	TObjectPtr<UObject> DraggedPayload;

	bool bIsDrawDropLocation;

public:
	// 인벤토리 내 아이템 Widget 갱신
	void Refresh();

protected:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, 
		const FGeometry& AllottedGeometry, 
		const FSlateRect& MyCullingRect, 
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	// Grid Inventory 내 item 및 ItemWidget 위치 변경
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	// 틱은 아니지만 해당 Widget 내에서 마우스를 드래그한 상태로 움직이면 계속 호출
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	bool IsRoomAvailableForPayload(AZNInventoryTestBaseItem* Item) const;

	FMousePositionInTile MousePositionInTileResult(FVector2D MousePosition);

	void CreateLineSegments();		// 그리드 선을 그릴 때 사용할 좌표를 구하는 함수
	void DrawBackgroundBox(AZNInventoryTestBaseItem* Item, FLinearColor MyTintColor, const FGeometry& AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayedId) const;

};
