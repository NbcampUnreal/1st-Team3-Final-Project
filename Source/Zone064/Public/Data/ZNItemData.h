#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ZNItemData.generated.h"

/*
* 모든 아이템의 기본 정보를 담는 Primary Data Asset
*/

USTRUCT(BlueprintType)
struct FItemPickupDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UZNItemData> ItemData;
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "없음"),
	Weapon UMETA(DisplayName = "무기"),
	Armor UMETA(DisplayName = "방어구"),
	Crafting UMETA(DisplayName = "제작"),
	Consumable UMETA(DisplayName = "소비")
};

UENUM(BlueprintType)
enum class EConsumableSubType : uint8
{
	None UMETA(DisplayName = "없음"),
	Health UMETA(DisplayName = "체력"),
	Hunger UMETA(DisplayName = "포만감")
};

UCLASS(BlueprintType)
class ZONE064_API UZNItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UZNItemData();
	
	/*
	* --- Item Data Info ---
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStackSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "-1"))
	int32 Durability;

	/*
	* --- Pickup Item Info ---
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName PickupRowName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FVector PickupScale = FVector(1.0f, 1.0f, 1.0f);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class AZNWeaponBase> HoldableWeaponClass;

	/*
	* --- Consumable Item Info ---
	*/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::Consumable"))       
	EConsumableSubType ConsumableSubType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable", meta = (EditCondition = "ItemType == EItemType::Consumable"))       
	float ConsumableEffectAmount;
	
	/*
	* --- UI ---
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	UTexture2D* ItemIcon;

public:
	/*
	* --- Primary Asset Id Functions ---
	*/
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FPrimaryAssetId GetItemPrimaryAssetId() const;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType("Item"), GetFName());
	}

	/*
	* --- Stack Utility Functions ---
	*/

	// 스택 가능한지 확인
	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsStackable() const;

	// 유효한 스택 크기 반환 (최소 1)
	UFUNCTION(BlueprintPure, Category = "Item") 
	int32 GetEffectiveStackSize() const;
};
