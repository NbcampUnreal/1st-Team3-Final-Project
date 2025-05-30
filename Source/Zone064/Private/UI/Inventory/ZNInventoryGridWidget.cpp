#include "UI/Inventory/ZNInventoryGridWidget.h"
#include "UI/Inventory/ZNItemWidget.h"
#include "Character/Inventory/ZNInventoryDataStruct.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Character/Inventory/ZNInventoryTestCharacter.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UZNInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 캐릭터 가져오기
	CharacterReference = Cast<AZNInventoryTestCharacter>(GetOwningPlayerPawn());
	InventoryComponent = CharacterReference->InventoryComponent;

	if (!CharacterReference)
		return;

	Columns = InventoryComponent->Columns;
	Rows = InventoryComponent->Rows;
	TileSize = InventoryComponent->TileSize;

	float NewWidth = Columns * TileSize;
	float NewHeight = Rows * TileSize;

	LineStructData = new FLines();
	StartX = {};
	StartY = {};
	EndX = {};
	EndY = {};

	UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	BorderAsCanvasSlot->SetSize(FVector2D(NewWidth, NewHeight));

	// GridBorder의 Width와 Height를 변경
	//UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	//BorderAsCanvasSlot->SetSize(FVector2D(Width, Height));

	CreateLineSegments();

	InventoryComponent->SetInventoryGridWidget(this);

	SetIsFocusable(true);
	bDropped = false;
	bIsDrawDropLocation = false;
}

int32 UZNInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// UI 선을 그리기 위해 필요한 Context
	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	// 선의 custom 색상
	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.5f);	// R, G, B, A(불투명도)

	// 인벤토리 그리드 왼쪽 상단 위치
	// - Border의 로컬 (0, 0)이 왼쪽 상단임, 이 위치를 전체 화면에서의 위치로 변경해서 반환
	FVector2D TopLeftCorner = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));

	if (LineStructData)
	{
		for (int32 i = 0; i < LineStructData->XLines.Num(); i++)
		{
			UWidgetBlueprintLibrary::DrawLine(PaintContext, FVector2D(StartX[i], StartY[i]) + TopLeftCorner, FVector2D(EndX[i], EndY[i]) + TopLeftCorner, CustomColor, false, 2.0f);
		}
	}

	if (bIsDrawDropLocation)
	{
		AZNInventoryTestBaseItem* Item = Cast<AZNInventoryTestBaseItem>(DraggedPayload);

		if (IsRoomAvailableForPayload(Item))
		{
			DrawBackgroundBox(Item, FLinearColor(0.0f, 1.0f, 0.0f, 0.25f), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
		}
		else
		{
			DrawBackgroundBox(Item, FLinearColor(1.0f, 0.0f, 0.0f, 0.25f), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
		}
	}

	return int32();
}

bool UZNInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (InOperation->Payload)
	{
		DroppedItem = Cast<AZNInventoryTestBaseItem>(InOperation->Payload);
		if (DroppedItem)
		{
			// Drop한 위치에 아이템을 넣을 수 있는지
			if (IsRoomAvailableForPayload(DroppedItem))
			{
				InventoryComponent->RefreshAllItems();
				InventoryComponent->AddItemAt(DroppedItem, InventoryComponent->TileToIndex(DraggedItemTopLeftTile));
			}
			else
			{
				// 못 넣는 경우
				// - 1. 일단 인벤토리에 다시 넣음(아이템이 들어갈 수 있는 가장 빠른 index에)
				InventoryComponent->RefreshAllItems();
				if (!InventoryComponent->TryAddItem(DroppedItem))
				{
					// - 2. 이것도 안되면 그냥 바닥에 뿌림
					FVector SpawnLocation = CharacterReference->GetActorLocation() * FVector(1.0f, 1.0f, 0.0f) + CharacterReference->GetActorForwardVector() * 100.0f;
					FRotator SpawnRotation = CharacterReference->GetActorRotation();

					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					AZNInventoryTestBaseItem* SpawnedItem = GetWorld()->SpawnActor<AZNInventoryTestBaseItem>(InOperation->Payload->GetClass(), SpawnLocation, SpawnRotation, SpawnParams);
				}
			}

			bDropped = true;
			bIsDrawDropLocation = false;
			return true;
		}
	}

	return false;
}

// 드래그한 ItemWidget의 TopLeft Tile 구하기
bool UZNInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (InOperation->Payload)
	{
		DraggedItem = Cast<AZNInventoryTestBaseItem>(InOperation->Payload);
		if (DraggedItem)
		{
			FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();	// 실제 화면에서의 마우스 커서 위치
			FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);	// 실제 마우스 좌표를 위젯 로컬 좌표로 변경

			// LocalPosition은 부모 위젯이나 다른 요소들에 의해 기준점이 바뀔 수 있음
			FVector2D GridStarterCoordinates = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.0f, 0.0f));	// GridBorder의 TopLeft좌표
			FVector2D AdjustedPosition = LocalPosition - GridStarterCoordinates;

			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("LOCAL -  X : %.2f, Y : %.2f"), LocalPosition.X, LocalPosition.Y));
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("ADJUST - X : %.2f, Y : %.2f"), AdjustedPosition.X, AdjustedPosition.Y));

			FIntPoint ResultTile;
			bool Down_ = MousePositionInTileResult(AdjustedPosition).Down;
			bool Right_ = MousePositionInTileResult(AdjustedPosition).Right;

			if (Right_)
			{
				// 클램프를 왜 한 건지 잘 모르겠음;;
				// ResultTile.X = DraggedItem->GetDimensions().X - 1; 이렇게만 해도 괜찮을 것 같음
				ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X - 1, 0, DraggedItem->GetDimensions().X - 1);
			}
			else
			{
				ResultTile.X = FMath::Clamp(DraggedItem->GetDimensions().X, 0, DraggedItem->GetDimensions().X);
			}

			if (Down_)
			{
				ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y - 1, 0, DraggedItem->GetDimensions().Y - 1);
			}
			else
			{
				ResultTile.Y = FMath::Clamp(DraggedItem->GetDimensions().Y, 0, DraggedItem->GetDimensions().Y);
			}

			// ItemWidget을 드래그하면 마우스는 ItemWidget의 정중앙에 위치하게 됨
			// - 마우스 위치는 그대로지만 Widget이 마우스가 중앙에 가도록 위치가 바뀜
			// => 마우스 위치에서 ItemWidget 크기의 절반만큼 빼면 ItemWidget의 TopLeft 위치를 구할 수 있음
			DraggedItemTopLeftTile = FIntPoint(FMath::TruncToInt32(AdjustedPosition.X / InventoryComponent->TileSize),
				FMath::TruncToInt32(AdjustedPosition.Y / InventoryComponent->TileSize)) - (ResultTile / 2);

			return true;
		}
	}

	return false;
}

// ItemWidget 회전
FReply UZNInventoryGridWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::R)
	{
		if (DraggedItem)
		{
			DraggedItem->RotateItem();

			if (StoredDragOperation)
			{
				UZNItemWidget* VisualDraggedItem = Cast<UZNItemWidget>(StoredDragOperation->DefaultDragVisual);
				if (VisualDraggedItem)
				{
					VisualDraggedItem->Refresh(DraggedItem);
				}

				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UZNInventoryGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	bIsDrawDropLocation = true;
	DraggedPayload = InOperation->Payload;

	// 여기서 받는 InOperation은 ItemWidget의 NativeOnDragDetected에서 설정한 OutOperation을 받는 듯
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(InOperation);
	if (DragOperation)
	{
		StoredDragOperation = DragOperation;
	}
}

void UZNInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	DraggedPayload = nullptr;
	bIsDrawDropLocation = false;
}

bool UZNInventoryGridWidget::IsRoomAvailableForPayload(AZNInventoryTestBaseItem* Item) const
{
	if (Item)
	{
		return InventoryComponent->IsRoomAvailable(Item, InventoryComponent->TileToIndex(DraggedItemTopLeftTile));
	}

	return false;
}

FMousePositionInTile UZNInventoryGridWidget::MousePositionInTileResult(FVector2D MousePosition)
{
	// fmod() : 실수용 % 연산 함수
	// 타일 한 칸에서의 MousePosition
	MousePositionInTile.Right = fmod(MousePosition.X, InventoryComponent->TileSize) > (InventoryComponent->TileSize / 2);
	MousePositionInTile.Down = fmod(MousePosition.Y, InventoryComponent->TileSize) > (InventoryComponent->TileSize / 2);

	return MousePositionInTile;
}

// Start, End TArray 채우기 = 격자 줄에 대한 좌표 구하기
void UZNInventoryGridWidget::CreateLineSegments()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("Columns : %d, Rows : %d"), Columns, Rows));

	// 세로 줄(x = columns)
	for (int32 i = 0; i <= Columns; i++)
	{
		float x = i * TileSize;	// float x(i * TileSize);

		LineStructData->XLines.Add(FVector2D(x, x));					// FVector2D의 X = 시작 좌표의 X 값 == Y = 끝 좌표의 X 값
		LineStructData->YLines.Add(FVector2D(0.0f, Rows * TileSize));	// FVector2D의 X = 시작 좌표의 Y 값,   Y = 끝 좌표의 Y 값
																		// => x좌표는 같고, y좌표는 다름 == (0, 0) ~ (0, 10) => 세로 줄
	}

	// 가로 줄(y = rows)
	for (int32 i = 0; i <= Rows; i++)
	{
		float y = i * TileSize;

		LineStructData->XLines.Add(FVector2D(0.0f, Columns * TileSize));
		LineStructData->YLines.Add(FVector2D(y, y));
	}

	for (const FVector2D Element : LineStructData->XLines)
	{
		StartX.Add(Element.X);
		EndX.Add(Element.Y);
	}

	for (const FVector2D Element : LineStructData->YLines)
	{
		StartY.Add(Element.X);
		EndY.Add(Element.Y);
	}

	//for (int32 i = 0; i < LineStructData->XLines.Num(); i++)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("StartX : %.2f, StartY : %.2f"), StartX[i], StartY[i]));
	//	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, FString::Printf(TEXT("EndX : %.2f, EndY : %.2f"), EndX[i], EndY[i]));
	//}
}

void UZNInventoryGridWidget::DrawBackgroundBox(AZNInventoryTestBaseItem* Item, FLinearColor MyTintColor, const FGeometry& AllottedGeometry,
												FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayedId) const
{
	FSlateBrush BoxBrush;
	BoxBrush.DrawAs = ESlateBrushDrawType::Box;

	FVector2D BoxSize(Item->GetDimensions().X * TileSize, Item->GetDimensions().Y * TileSize);
	FIntPoint BoxPosition(DraggedItemTopLeftTile.X * TileSize, DraggedItemTopLeftTile.Y * TileSize);

	FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(TopLeftCorner + BoxPosition));

	FSlateDrawElement::MakeBox(OutDrawElements, LayedId, PaintGeometry, &BoxBrush, ESlateDrawEffect::None, MyTintColor);
}

void UZNInventoryGridWidget::Refresh()
{
	TArray<AZNInventoryTestBaseItem*> Keys;
	InventoryComponent->GetAllItems().GetKeys(Keys);

	if (CharacterReference->ItemWidgetClass)
	{
		CharacterReference->ItemWidgetInstance = CreateWidget(GetWorld(), CharacterReference->ItemWidgetClass);
		
		for (AZNInventoryTestBaseItem* AddedItem : Keys)
		{
			CharacterReference->ItemWidgetInstance->SetOwningPlayer(GetOwningPlayer());
			int32 X = InventoryComponent->GetAllItems()[AddedItem].X * InventoryComponent->TileSize;
			int32 Y = InventoryComponent->GetAllItems()[AddedItem].Y * InventoryComponent->TileSize;

			PanelSlot = GridCanvasPanel->AddChild(CharacterReference->ItemWidgetInstance);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
			Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(X,Y));
		}
	}
}