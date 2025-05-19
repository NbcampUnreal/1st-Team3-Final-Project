#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZNItemWidget.generated.h"

class UCanvasPanel;
class USizeBox;
class UBorder;
class UImage;

UCLASS()
class ZONE064_API UZNItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UCanvasPanel* Canvas;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	USizeBox* BackgroundSizebox;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBorder* BackgroundBorder;
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UImage* ItemImage;
};
