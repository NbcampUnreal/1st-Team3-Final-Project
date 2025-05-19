#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNItemWidget.generated.h"

class UCanvasPanel;
class USizeBox;
class UBorder;
class UImage;
class AZNInventoryTestCharacter;

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

	TObjectPtr<AZNInventoryTestCharacter> CharacterReference;

	FVector2D Size;

protected:
	virtual void NativeConstruct() override;

	// 아이템 위젯의 세팅 조정(아이콘, 크기 등)
	void Refresh(AActor* ItemToAdd);
};
