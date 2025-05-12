#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNInventoryGridWidget.generated.h"

class UCanvasPanel;
class UBorder;
struct FLines;

UCLASS()
class ZONE064_API UZNInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> Canvas;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBorder> GridBorder;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> GridCanvasPanel;

	// ���߿� ĳ���� ������Ÿ�� �����ǰ� ���� UPROPERTY(), Width, Height �����
	// NativeConstruct()�� �ִ� �ּ� Ǯ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Columns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Rows;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float TileSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float Width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float Height;

	TSharedPtr<FLines> LineStructData;

	TArray<float> StartX;
	TArray<float> StartY;
	TArray<float> EndX;
	TArray<float> EndY;

protected:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, 
		const FGeometry& AllottedGeometry, 
		const FSlateRect& MyCullingRect, 
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	// �׸��� ���� �׸� �� ����� ��ǥ�� ���ϴ� �Լ�
	void CreateLineSegments();

	
};
