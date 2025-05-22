#include "UI/Inventory/ZNItemWidget.h"
#include "UI/Inventory/ZNInventoryGridWidget.h"
#include "Character/Inventory/ZNInventoryTestCharacter.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "Components/Image.h"
#include "Components/Sizebox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Kismet/GameplayStatics.h"

void UZNItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterReference = Cast<AZNInventoryTestCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (CharacterReference->InventoryComponent->InventoryGridWidgetReference->bDropped)
	{
		Refresh(CharacterReference->InventoryComponent->InventoryGridWidgetReference->DroppedItem);
	}
	else if (CharacterReference)
	{
		Refresh(CharacterReference->ItemToAdd);
	}
}
 
// hover in
void UZNItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.2f));
}

// hover out
void UZNItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	BackgroundBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
}

void UZNItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	BackgroundBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));

	UDragDropOperation* DragOperation = NewObject<UDragDropOperation>();
	DragOperation->DefaultDragVisual = this;	// the widget to display when dragging the item
	DragOperation->Payload = Item;				// 드래그하는 동안 Widget의 Item을 저장
	OutOperation = DragOperation;

	CharacterReference->InventoryComponent->RemoveItem(Item);

	this->RemoveFromParent();
}

// 마우스 버튼 동작 감지 - Drag 감지
FReply UZNItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

void UZNItemWidget::Refresh(AActor* ItemToAdd)
{
	Item = Cast<AZNInventoryTestBaseItem>(ItemToAdd);

	// 캐릭터는 먹은 Item에 대해 저장을 해놓고
	// 이 Item의 정보를 가지고 ItemWidget을 만들어야함
	// 1. 위젯 이미지 지정
	if (!Item->GetIsRotated())
	{
		ItemImage->SetBrushFromMaterial(Item->GetIcon());
	}
	else
	{
		ItemImage->SetBrushFromMaterial(Item->GetRotatedIcon());
	}

	// 2. 위젯 크기 지정
	Size = FVector2D(Item->GetDimensions().X * CharacterReference->InventoryComponent->TileSize, Item->GetDimensions().Y * CharacterReference->InventoryComponent->TileSize);
	BackgroundSizebox->SetWidthOverride(Size.X);
	BackgroundSizebox->SetHeightOverride(Size.Y);

	// 3. 위젯 크기에 맞게 위젯 이미지 크기 조정
	// - 지금 보니까 GridWidget에서도 그렇고, 어떤 Component의 크기를 직접 조절하지 않고
	// - CanvasPanelSlot을 붙여서 조절함
	UCanvasPanelSlot* ImageAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage);
	ImageAsCanvasSlot->SetSize(Size);
}
