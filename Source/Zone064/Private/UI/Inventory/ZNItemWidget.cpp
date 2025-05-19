#include "UI/Inventory/ZNItemWidget.h"
#include "Character/Inventory/ZNInventoryTestCharacter.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "Components/Image.h"
#include "Components/Sizebox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

void UZNItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterReference = Cast<AZNInventoryTestCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (CharacterReference)
	{
		Refresh(CharacterReference->ItemToAdd);
	}
}

void UZNItemWidget::Refresh(AActor* ItemToAdd)
{
	AZNInventoryTestBaseItem* Item = Cast<AZNInventoryTestBaseItem>(ItemToAdd);

	// 캐릭터는 먹은 Item에 대해 저장을 해놓고
	// 이 Item의 정보를 가지고 ItemWidget을 만들어야함
	// 1. 위젯 이미지 지정
	ItemImage->SetBrushFromMaterial(Item->GetIcon());

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
