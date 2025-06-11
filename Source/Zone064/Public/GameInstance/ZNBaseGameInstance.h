// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "ZNBaseGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UZNBaseGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	/* Data Tables */
	UPROPERTY(EditDefaultsOnly)
	UDataTable* MapNameDataTable;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* WidgetDataTable;
};
