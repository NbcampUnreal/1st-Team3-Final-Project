// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "GameSystems/Types/TravelTypes.h"
#include "GameSystems/Datas/MapDataRow.h"
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
	void ChangeGamePhase(EGamePhase _NextGamePhase);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ChangeMapByName(FName _NextMapName, bool _bServerTravel);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ChangeMapByPhase(EGamePhase _NextGamePhase, bool _bServerTravel);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void ChangePhaseAndMap(EGamePhase _NextGamePhase, bool _bServerTravel);
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void RequestPhaseTransition(EGamePhase _NextGamePhase, ELevelTravelType _TravelType);

	/* Getter, Setter, ... etc. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EGamePhase GetCurrentGamePhase();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetCurrentMapName();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	int32 GetCurrentRepeatCount();
	/*UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetInternalMapNameByPhase(EGamePhase _GamePhase);*/
	void SetCurrentGamePhase(EGamePhase _GamePhase);
	void SetCurrentMapName(FName _MapName);
	void InitCurrentRepeatCount();
	void AddCurrentRepeatCount();

private:
	/* GameFlow Data */
	EGamePhase CurrentGamePhase;
	FName CurrentMapName;
	int32 CurrentRepeatCount;
	UPROPERTY(EditDefaultsOnly)
	int32 MaxRepeatCount = 4;

	/* MapName Data */
	TMap<EGamePhase, FMapDataRow> MapDataCache;
};	
