#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNInventoryGridWidget.generated.h"

class UCanvasPanel;
class UBorder;
struct FLines;
class UZNInventoryComponent;
class AZNInventoryTestCharacter;

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

	TObjectPtr<AZNInventoryTestCharacter> CharacterReference;
	TObjectPtr<UZNInventoryComponent> InventoryComponent;
	
	// ItemWidget을 붙일 PanelSlot
	TObjectPtr<UPanelSlot> PanelSlot;


	// 나중에 캐릭터 프로토타입 구현되고 나면 UPROPERTY(), Width, Height 지우고
	// NativeConstruct()에 있는 주석 풀기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Columns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	int32 Rows;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float TileSize;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	//float Width;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	//float Height;

	TSharedPtr<FLines> LineStructData;

	TArray<float> StartX;
	TArray<float> StartY;
	TArray<float> EndX;
	TArray<float> EndY;

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

	// 그리드 선을 그릴 때 사용할 좌표를 구하는 함수
	void CreateLineSegments();

};
