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
    // ������ ���� �̸� ���� + GameInstance�� ����
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static FString GenerateUniqueSessionName(UObject* WorldContextObject);

    // ����� ���� �̸� ��� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static void SafeDestroySession(UObject* WorldContextObject);

    // ����� ���� �̸� ��ȸ
    UFUNCTION(BlueprintCallable, Category = "Session", meta = (WorldContext = "WorldContextObject"))
    static FName GetCurrentSessionName(UObject* WorldContextObject);

    // ���� ���͸�
    UFUNCTION(BlueprintCallable, Category = "Session")
    static TArray<FBlueprintSessionResult> FilterValidSessions(const TArray<FBlueprintSessionResult>& SessionResults);

    // ���� ����
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool CreateFullSession(UObject* WorldContextObject, int32 MaxPlayers, const FString& GameName);

    // ���� ����
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool JoinNamedSession(UObject* WorldContextObject, const FBlueprintSessionResult& SearchResult);

    // ��ư ���� Ŭ�� ����
    UFUNCTION(BlueprintCallable, Category = "UI|Utils")
    static void TemporarilyDisableButton(UButton* TargetButton, float DisableDuration = 1.0f);

    // ���� �ο��� ����
    UFUNCTION(BlueprintPure, Category = "Session")
    static FText GetFormattedSessionPlayerCount(const FBlueprintSessionResult& SessionResult);

    UFUNCTION(BlueprintCallable, Category = "Session")
    static void UpdatePlayerCountInSession(UObject* WorldContextObject, int32 Delta);
};
