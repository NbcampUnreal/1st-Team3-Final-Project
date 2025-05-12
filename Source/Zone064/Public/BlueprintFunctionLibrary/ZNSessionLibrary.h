// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintDataDefinitions.h"
#include "Components/Button.h"
#include "ZNSessionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UZNSessionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    // 고유한 세션 이름 생성 + GameInstance에 저장
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static FString GenerateUniqueSessionName(UObject* WorldContextObject);

    // 저장된 세션 이름 기반 세션 제거
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static void SafeDestroySession(UObject* WorldContextObject);

    // 저장된 세션 이름 조회
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static FName GetCurrentSessionName(UObject* WorldContextObject);

    // 세션 필터링
    UFUNCTION(BlueprintCallable, Category = "Session")
    static TArray<FBlueprintSessionResult> FilterValidSessions(const TArray<FBlueprintSessionResult>& SessionResults);

    // 세션 생성
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool CreateFullSession(UObject* WorldContextObject, int32 MaxPlayers, const FString& GameName);

    // 세션 참가
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool JoinNamedSession(UObject* WorldContextObject, const FBlueprintSessionResult& SearchResult);

    // 버튼 연속 클릭 방지
    UFUNCTION(BlueprintCallable, Category = "UI|Utils")
    static void TemporarilyDisableButton(UButton* TargetButton, float DisableDuration = 1.0f);

    // 세션 인원수 적용
    UFUNCTION(BlueprintPure, Category = "Session")
    static FText GetFormattedSessionPlayerCount(const FBlueprintSessionResult& SessionResult);

    UFUNCTION(BlueprintCallable, Category = "Session")
    static void UpdatePlayerCountInSession(UObject* WorldContextObject, int32 Delta);
};
