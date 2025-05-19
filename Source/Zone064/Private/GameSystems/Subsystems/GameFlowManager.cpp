// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystems/Subsystems/GameFlowManager.h"
#include "Kismet/GameplayStatics.h"


void UGameFlowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGamePhase = EGamePhase::Title;
}

void UGameFlowManager::AdvanceGamePhase()
{
	switch (CurrentGamePhase)
	{
	case EGamePhase::Title:
	{
		GoToMenuPhase();
		break;
	}
	case EGamePhase::Menu:
	{
		GoToLobbyPhase();
		break;
	}
	case EGamePhase::Lobby:
	{
		GoToDeparturePhase();
		break;
	}
	case EGamePhase::Departure:
	{
		GoToDrivingPhase();
		break;
	}
	case EGamePhase::Driving:
	{
		GoToInGamePhase();
		break;
	}
	case EGamePhase::InGame:
	{
		GoToCampingPhase();
		break;
	}
	case EGamePhase::Camping:
	{
		GoToVotingPhase();
		break;
	}
	case EGamePhase::Voting:
	{
		GoToDefensePhase();
		break;
	}
	case EGamePhase::Defense:
	{
		GoToEndingPhase();
		break;
	}
	case EGamePhase::Ending:
	{
		GoToReturnPhase();
		break;
	}
	case EGamePhase::ReturnToTitle:
	{
		GoToTitlePhase();
		break;
	}
	default:
	{
		break;
	}
	}
}

void UGameFlowManager::GoToTitlePhase()
{
	CurrentGamePhase = EGamePhase::Title;
	UGameplayStatics::OpenLevel(this, FName("TitleLevel"));		// todo: Server, Client differentiate?
}

void UGameFlowManager::GoToMenuPhase()
{
	CurrentGamePhase = EGamePhase::Menu;
	UGameplayStatics::OpenLevel(this, FName("MenuLevel"));
}

void UGameFlowManager::GoToLobbyPhase()
{
	CurrentGamePhase = EGamePhase::Lobby;
	UGameplayStatics::OpenLevel(this, FName("LobbyLevel"));
}

void UGameFlowManager::GoToDeparturePhase()
{
	CurrentGamePhase = EGamePhase::Departure;
	UGameplayStatics::OpenLevel(this, FName("DepartureLevel"));	// todo: Change to ServerTravel
}

void UGameFlowManager::GoToDrivingPhase()
{
	CurrentGamePhase = EGamePhase::Driving;
	UGameplayStatics::OpenLevel(this, FName("DrivingLevel"));	// todo: Change to ServerTravel
}

void UGameFlowManager::GoToInGamePhase()
{
	CurrentGamePhase = EGamePhase::InGame;
	UGameplayStatics::OpenLevel(this, FName("InGameLevel"));	// todo: Change to ServerTravel
}

void UGameFlowManager::GoToCampingPhase()
{
	CurrentGamePhase = EGamePhase::Camping;
	UGameplayStatics::OpenLevel(this, FName("CampingLevel"));	// todo: Same as InGameLevel?
}

void UGameFlowManager::GoToVotingPhase()
{
	CurrentGamePhase = EGamePhase::Voting;
	// No Level Change
}

void UGameFlowManager::GoToDefensePhase()
{
	CurrentGamePhase = EGamePhase::Defense;
	UGameplayStatics::OpenLevel(this, FName("DefenseLevel"));	// todo: Change to ServerTravel
}

void UGameFlowManager::GoToEndingPhase()
{
	CurrentGamePhase = EGamePhase::Ending;
	UGameplayStatics::OpenLevel(this, FName("EndingLevel"));	// todo: Change to ServerTravel
}

void UGameFlowManager::GoToReturnPhase()
{
	CurrentGamePhase = EGamePhase::ReturnToTitle;
	// No Level Change
}

