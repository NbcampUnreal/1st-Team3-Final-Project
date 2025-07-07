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

	/* Update GameState */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void UpdateGameFlowData();

	/* Repeat Count Methods */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void InitCurrentRepeatCount();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void AddCurrentRepeatCount();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	bool CheckMaxRepeatCount();

	/* Destination Methods */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void InitPastDestinationsCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void AddPastDestinationCache(FName _PastDestination);

	/* Getter, Setter */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EGamePhase GetCurGamePhaseCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EGamePhase GetPrevGamePhaseCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	int32 GetCurRepeatCountCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetCurDestinationNumCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EMapType GetCurDestinationTypeCache();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FDestinationDataRow GetDestDataCacheRow(FName _RowName) const;
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	TArray<FName> GetPastDestinationsCache();

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void SetCurDestinationNumCache(FName _DestinationNum);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void SetCurDestinationTypeCache(EMapType _DestinationType);

private:
	/* GameFlow Data Cache*/
	EGamePhase CurGamePhaseCache;
	EGamePhase PrevGamePhaseCache;

	FName CurMapNameCache;

	int32 CurRepeatCountCache;
	UPROPERTY(EditDefaultsOnly)
	int32 MaxRepeatCount = 4;

	FName CurDestinationNumCache;
	EMapType CurDestinationTypeCache;
	TArray<FName> PastDestinationsCache;

	/* DataTable Cache */
	TMap<EGamePhase, TArray<FMapDataRow>> MapDataCache;
	TMap<FName, FDestinationDataRow> DestDataCache;
};	
