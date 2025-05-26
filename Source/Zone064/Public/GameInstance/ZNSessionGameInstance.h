// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "ZNSessionGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UZNSessionGameInstance : public UZNBaseGameInstance
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadWrite)
    FName CurrentSessionName;
public:
    virtual void Init() override;

    // 종료하거나 나갔을 때 세션과의 연결 끊음
    virtual void Shutdown() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Init")
    void OnInit();

    UFUNCTION(BlueprintImplementableEvent, Category = "Network Error")
    void OnNetworkError(ENetworkFailure::Type FailureType, bool bIsServer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Travel Error")
    void OnTravelError(ETravelFailure::Type FailureType);

private:
    UFUNCTION()
    void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

    UFUNCTION()
    void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
};
