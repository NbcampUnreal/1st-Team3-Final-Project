#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNItemWidget.generated.h"

class UCanvasPanel;
class USizeBox;
class UBorder;
class UImage;
class AZNInventoryTestCharacter;
class AZNInventoryTestBaseItem;

UCLASS()
class ZONE064_API UZNItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UCanvasPanel> Canvas;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<USizeBox> BackgroundSizebox;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UBorder> BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<AZNInventoryTestCharacter> CharacterReference;

	FVector2D Size;

	TObjectPtr<AZNInventoryTestBaseItem> Item;

public:
	// 아이템 위젯의 세팅 조정(아이콘, 크기 등)
	void Refresh(AActor* ItemToAdd);

protected:
	virtual void NativeConstruct() override;

	// Hover
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	// Drag & Drop
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
