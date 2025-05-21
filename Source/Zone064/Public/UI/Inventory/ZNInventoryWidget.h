#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNInventoryWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UBackgroundBlur;
class AZNInventoryTestCharacter;
class AZNInventoryTestBaseItem;

UCLASS()
class ZONE064_API UZNInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> Canvas;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBorder> BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBackgroundBlur> Blur;

	TObjectPtr<AZNInventoryTestBaseItem> SpawnedItem;

protected:
	TObjectPtr<AZNInventoryTestCharacter> CharacterReference;

protected:
	virtual void NativeConstruct() override;

	// Inventory Widget을 만들 때 전체 화면으로 만듦
	// - Grid Inventory 밖에서 Drop한 경우
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
