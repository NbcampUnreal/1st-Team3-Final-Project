// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "ZNSessionGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UZNSessionGameInstance : public UZNBaseGameInstance
{
	GENERATED_BODY()
	
public:
	// 연결중인 세션 이름
	UPROPERTY(BlueprintReadWrite)
	FName CurrentSessionName;

	// 종료하거나 나갔을 때 세션과의 연결 끊음
	virtual void Shutdown() override;
};
