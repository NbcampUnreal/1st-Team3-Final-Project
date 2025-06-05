// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/ZNBaseGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameSystems/Subsystems/GameFlowManager.h"

void AZNBaseGameState::BeginPlay()
{
	Super::BeginPlay();

	if (UGameFlowManager* GameFlowManager = GetGameInstance()->GetSubsystem<UGameFlowManager>())
	{
		GameFlowManager->UpdateGameFlowData();
	}
}

EGamePhase AZNBaseGameState::GetCurrentGamePhase()
{
	return CurrentGamePhase;
}

FName AZNBaseGameState::GetCurrentMapName()
{
	return CurrentMapName;
}

int32 AZNBaseGameState::GetCurrentRepeatCount()
{
	return CurrentRepeatCount;
}

void AZNBaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZNBaseGameState, CurrentGamePhase);
	DOREPLIFETIME(AZNBaseGameState, CurrentMapName);
	DOREPLIFETIME(AZNBaseGameState, CurrentRepeatCount);
}

void AZNBaseGameState::SetCurrentGamePhase(EGamePhase _GamePhase)
{
	CurrentGamePhase = _GamePhase;
}

void AZNBaseGameState::SetCurrentMapName(FName _MapName)
{
	CurrentMapName = _MapName;
}

void AZNBaseGameState::SetCurrentRepeatCount(int32 _RepeatCount)
{
	CurrentRepeatCount = _RepeatCount;
}
