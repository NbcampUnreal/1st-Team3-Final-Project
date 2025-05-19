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
	
	void AdvanceGamePhase();

private:
	EGamePhase CurrentGamePhase;

	void GoToTitlePhase();
	void GoToMenuPhase();
	void GoToLobbyPhase();
	void GoToDeparturePhase();
	void GoToDrivingPhase();
	void GoToInGamePhase();
	void GoToCampingPhase();
	void GoToVotingPhase();
	void GoToDefensePhase();
	void GoToEndingPhase();
	void GoToReturnPhase();
};	
