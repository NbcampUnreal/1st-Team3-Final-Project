#include "UI/Inventory/ZNInventoryGridWidget.h"
#include "Character/Inventory/ZNInventoryDataStruct.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UZNInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ĳ���� ��������
	//TObjectPtr<ACharacter> CharacterReference;
	//CharacterReference = Cast<ACharacter>(GetOwningPlayerPawn());

	//TObjectPtr<UZNInventoryComponent> InventoryComponent;
	//InventoryComponent = CharacterReference->InventoryComponent;

	//if (!CharacterReference)
	//	return;

	//Columns = InventoryComponent->Columns;
	//Rows = InventoryComponent->Rows;
	//TileSize = InventoryComponent->TileSize;

	//float NewWidth = Columns * TileSize;
	//float NewHeight = Rows * TileSize;

	LineStructData = MakeShared<FLines>();
	StartX = {};
	StartY = {};
	EndX = {};
	EndY = {};

	//UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	//BorderAsCanvasSlot->SetSize(FVector2D(NewWidth, NewHeight));

	// GridBorder�� Width�� Height�� ����
	UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	BorderAsCanvasSlot->SetSize(FVector2D(Width, Height));

	CreateLineSegments();
}

int32 UZNInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// UI ���� �׸��� ���� �ʿ��� Context
	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	// ���� custom ����
	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.5f);	// R, G, B, A(������)

	// �κ��丮 �׸��� ���� ��� ��ġ
	// - Border�� ���� (0, 0)�� ���� �����, �� ��ġ�� ��ü ȭ�鿡���� ��ġ�� �����ؼ� ��ȯ
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

// Start, End TArray ä��� = ���� �ٿ� ���� ��ǥ ���ϱ�
void UZNInventoryGridWidget::CreateLineSegments()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString::Printf(TEXT("Columns : %d, Rows : %d"), Columns, Rows));

	// ���� ��(x = columns)
	for (int32 i = 0; i <= Columns; i++)
	{
		float x = i * TileSize;	// float x(i * TileSize);

		LineStructData->XLines.Add(FVector2D(x, x));					// FVector2D�� X = ���� ��ǥ�� X �� == Y = �� ��ǥ�� X ��
		LineStructData->YLines.Add(FVector2D(0.0f, Rows * TileSize));	// FVector2D�� X = ���� ��ǥ�� Y ��,   Y = �� ��ǥ�� Y ��
																		// => x��ǥ�� ����, y��ǥ�� �ٸ� == (0, 0) ~ (0, 10) => ���� ��
	}

	// ���� ��(y = rows)
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