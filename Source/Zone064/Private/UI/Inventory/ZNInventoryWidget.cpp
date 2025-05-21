#include "UI/Inventory/ZNInventoryWidget.h"
#include "Character/Inventory/ZNInventoryTestCharacter.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "Blueprint/DragDropOperation.h"
#include "Kismet/GameplayStatics.h"

void UZNInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterReference = Cast<AZNInventoryTestCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

FReply UZNInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

bool UZNInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (InOperation->Payload)
	{
		FVector SpawnLocation = CharacterReference->GetActorLocation() * FVector(1.0f, 1.0f, 0.0f) + CharacterReference->GetActorForwardVector() * 100.0f;
		FRotator SpawnRotation = CharacterReference->GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 내가 가지고 있던 완전 동일한 아이템이 Spawn되고 있지 않음
		// Why? Item의 Class Default에는 Dimension이랑 ICon 같은 것들이 설정되어 있지 않음
		// => 다시 먹었을 때 Inventory에서 보이지 않음
		SpawnedItem = GetWorld()->SpawnActor<AZNInventoryTestBaseItem>(InOperation->Payload->GetClass(), SpawnLocation, SpawnRotation, SpawnParams);

		// 버리고 나면 InventoryComponent의 AllItems 갱신해야함

		return true;
	}
	else
	{
		return false;
	}
}
