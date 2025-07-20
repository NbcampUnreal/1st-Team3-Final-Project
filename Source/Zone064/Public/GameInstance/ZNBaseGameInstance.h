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
	
	//UPROPERTY(EditDefaultsOnly)
	//UDataTable* WidgetDataTable;

	/* Utility Function */
	const FItemPickupDataRow* GetItemPickupDataRow(const FName& RowName) const;
	
	UFUNCTION(BlueprintCallable, Category = "ItemData")
	UZNItemData* GetItemDataFromPickup(const FName& RowName) const;

};
