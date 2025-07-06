#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ZNItemData.generated.h"

/*
* 모든 아이템의 기본 정보를 담는 Primary Data Asset
*/

UCLASS(BlueprintType)
class ZONE064_API UZNItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/*
	* --- Item Info ---
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMax = "1"))
	int32 MaxStackSize;

	/*
	* --- UI ---
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> ItemIcon;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType("Item"), GetFName());
	}
};
