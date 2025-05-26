// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "GameFlowManager.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UGameFlowManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/* Flow Control Methods */
	void AdvanceGamePhase();
	void ChangePhaseAndMap(EGamePhase _NextGamePhase);

	/* Getter, Setter, Init, Add, ... etc. */
	void SetCurrentGamePhase(EGamePhase _GamePhase);
	void SetCurrentMapName(FName _MapName);
	void InitCurrentRepeatCount();
	void AddCurrentRepeatCount();

private:
	/* Current Flow Datas */
	EGamePhase CurrentGamePhase;
	FName CurrentMapName;
	int32 CurrentRepeatCount;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxRepeatCount = 4;

	/* MapName Data Table */
	TMap<EGamePhase, FName> MapNameCache;
	//UDataTable* MapNameDataTable;
};	
