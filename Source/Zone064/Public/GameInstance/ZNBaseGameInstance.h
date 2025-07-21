#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Data/ZNItemData.h"
#include "ZNBaseGameInstance.generated.h"

UCLASS()
class ZONE064_API UZNBaseGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	/* Data Tables */
	UPROPERTY(EditDefaultsOnly)
	UDataTable* MapDataTable;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* DestDataTable;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemPickupDataTable;

	// UPROPERTY(EditDefaultsOnly)
	// UDataTable* WidgetDataTable;
	
	// 기본 아이템 픽업 액터 클래스 (모든 Storage에서 공통 사용)
	UPROPERTY(EditDefaultsOnly, Category = "ItemPickup")
	TSubclassOf<class AZNBasePickup> DefaultPickupActorClass;

	UPROPERTY(EditAnywhere, Category = "ItemData")
	int32 TotalFuelPerPhase = 10;
	
	/* Utility Function */
	const FItemPickupDataRow* GetItemPickupDataRow(const FName& RowName) const;
	
	UFUNCTION(BlueprintCallable, Category = "ItemData")
	UZNItemData* GetItemDataFromPickup(const FName& RowName) const;
};
