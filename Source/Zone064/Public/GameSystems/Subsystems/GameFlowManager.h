// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "GameSystems/Types/TravelTypes.h"
#include "GameSystems/Types/MapTypes.h"
#include "GameSystems/Datas/MapDataRow.h"
#include "GameSystems/Datas/DestinationDataRow.h"
#include "GameFlowManager.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ZONE064_API UGameFlowManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/* Flow Control Methods */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void RequestPhaseTransition(EGamePhase _NextGamePhase, ELevelTravelType _TravelType);

	/*UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetInternalMapNameByPhase(EGamePhase _GamePhase);*/

	/* Repeat Count Methods */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void InitCurrentRepeatCount();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void AddCurrentRepeatCount();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	bool CheckMaxRepeatCount();

	/* Update GameState */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void UpdateGameFlowData();

	/* Getter, Setter */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EGamePhase GetCurGamePhaseCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	int32 GetCurRepeatCountCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetCurDestinationNumCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EMapType GetCurDestinationTypeCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FDestinationDataRow GetDestDataCacheRow(FName _RowName) const;

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void SetCurDestinationNumCache(FName _DestinationNum);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void SetCurDestinationTypeCache(EMapType _DestinationType);

private:
	/* GameFlow Data Cache*/
	EGamePhase CurGamePhaseCache;
	FName CurMapNameCache;
	FName CurDestinationNumCache;
	EMapType CurDestinationTypeCache;
	int32 CurRepeatCountCache;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxRepeatCount = 4;

	/* DataTable Cache */
	TMap<EGamePhase, TArray<FMapDataRow>> MapDataCache;
	TMap<FName, FDestinationDataRow> DestDataCache;
};	
