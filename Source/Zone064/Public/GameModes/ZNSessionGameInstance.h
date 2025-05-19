// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "ZNSessionGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UZNSessionGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite)
	FName CurrentSessionName;

	virtual void Shutdown() override;
};
