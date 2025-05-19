#include "UI/Inventory/ZNInventoryGridWidget.h"
#include "Character/Inventory/ZNInventoryDataStruct.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Character/Inventory/ZNInventoryTestCharacter.h"
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

	LineStructData = MakeShared<FLines>();
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

	if (LineStructData.IsValid())
	{
		for (int32 i = 0; i < LineStructData->XLines.Num(); i++)
		{
			UWidgetBlueprintLibrary::DrawLine(PaintContext, FVector2D(StartX[i], StartY[i]) + TopLeftCorner, FVector2D(EndX[i], EndY[i]) + TopLeftCorner, CustomColor, false, 2.0f);
		}
	}

	return int32();
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