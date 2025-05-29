// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystems/Subsystems/GameFlowManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "GameSystems/Datas/MapDataRow.h"


void UGameFlowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache Data Table
	UZNBaseGameInstance* GI = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (GI && GI->MapNameDataTable)
	{
		static const FString Context = TEXT("MapNameDT");
		for (const auto& RowName : GI->MapNameDataTable->GetRowNames())
		{
			const FMapDataRow* Row = GI->MapNameDataTable->FindRow<FMapDataRow>(RowName, Context);
			if (!Row)
			{
				continue;
			}

			if (!Row->MapName.IsNone())
			{
				MapNameCache.Add(Row->GamePhase, Row->MapName);
			}
		}
	}

	// Initialize GamePhase, Map
	CurrentGamePhase = EGamePhase::None;
	//AdvanceGamePhase();
}

void UGameFlowManager::AdvanceGamePhase()
{
	switch (CurrentGamePhase)
	{
	case EGamePhase::None:
	{
		InitCurrentRepeatCount();
		ChangePhaseAndMap(EGamePhase::Title);
		break;
	}
	case EGamePhase::Title:
	{
		ChangePhaseAndMap(EGamePhase::Menu);
		break;
	}
	case EGamePhase::Menu:
	{
		ChangePhaseAndMap(EGamePhase::Lobby);
		break;
	}
	case EGamePhase::Lobby:
	{
		ChangePhaseAndMap(EGamePhase::Departure);
		break;
	}
	case EGamePhase::Departure:
	{
		ChangePhaseAndMap(EGamePhase::Driving);
		break;
	}
	case EGamePhase::Driving:
	{
		ChangePhaseAndMap(EGamePhase::InGame);
		break;
	}
	case EGamePhase::InGame:
	{
		ChangePhaseAndMap(EGamePhase::Camping);
		break;
	}
	case EGamePhase::Camping:
	{
		ChangePhaseAndMap(EGamePhase::Voting);
		break;
	}
	case EGamePhase::Voting:
	{
		if (CurrentRepeatCount < MaxRepeatCount - 1)
		{
			AddCurrentRepeatCount();
			ChangePhaseAndMap(EGamePhase::Driving);	// next loop
		}
		else
		{
			ChangePhaseAndMap(EGamePhase::Defense);	// end loop
		}
		break;
	}
	case EGamePhase::Defense:
	{
		ChangePhaseAndMap(EGamePhase::Ending);
		break;
	}
	case EGamePhase::Ending:
	{
		ChangePhaseAndMap(EGamePhase::ReturnToTitle);
		break;
	}
	case EGamePhase::ReturnToTitle:
	{
		InitCurrentRepeatCount();
		ChangePhaseAndMap(EGamePhase::Title);
		break;
	}
	default:
	{
		break;
	}
	}
}

void UGameFlowManager::ChangePhaseAndMap(EGamePhase _NextGamePhase)
{
	// Change Map
	FName NextMapName = *MapNameCache.Find(_NextGamePhase);

	if(NextMapName != CurrentMapName)
	{
		SetCurrentMapName(NextMapName);
		UGameplayStatics::OpenLevel(this, NextMapName);	// todo: servertravel 해야 하는 경우 분리
	}

	// Change GamePhase
	SetCurrentGamePhase(_NextGamePhase);
}

EGamePhase UGameFlowManager::GetCurrentGamePhase()
{
	return CurrentGamePhase;
}

FName UGameFlowManager::GetCurrentMapName()
{
	return CurrentMapName;
}

int32 UGameFlowManager::GetCurrentRepeatCount()
{
	return CurrentRepeatCount;
}

void UGameFlowManager::SetCurrentGamePhase(EGamePhase _GamePhase)
{
	CurrentGamePhase = _GamePhase;
}

void UGameFlowManager::SetCurrentMapName(FName _MapName)
{
	CurrentMapName = _MapName;
}

void UGameFlowManager::InitCurrentRepeatCount()
{
	CurrentRepeatCount = 0;
}

void UGameFlowManager::AddCurrentRepeatCount()
{
	CurrentRepeatCount += 1;
}


//void UGameFlowManager::GoToTitlePhase()
//{
//	CurrentGamePhase = EGamePhase::Title;
//	UGameplayStatics::OpenLevel(this, FName("TitleLevel"));		// todo: Server, Client differentiate?
//}
//
//void UGameFlowManager::GoToMenuPhase()
//{
//	CurrentGamePhase = EGamePhase::Menu;
//	UGameplayStatics::OpenLevel(this, FName("MenuLevel"));
//}
//
//void UGameFlowManager::GoToLobbyPhase()
//{
//	CurrentGamePhase = EGamePhase::Lobby;
//	UGameplayStatics::OpenLevel(this, FName("LobbyLevel"));
//}
//
//void UGameFlowManager::GoToDeparturePhase()
//{
//	CurrentGamePhase = EGamePhase::Departure;
//	UGameplayStatics::OpenLevel(this, FName("DepartureLevel"));	// todo: Change to ServerTravel
//}
//
//void UGameFlowManager::GoToDrivingPhase()
//{
//	CurrentGamePhase = EGamePhase::Driving;
//	UGameplayStatics::OpenLevel(this, FName("DrivingLevel"));	// todo: Change to ServerTravel
//}
//
//void UGameFlowManager::GoToInGamePhase()
//{
//	CurrentGamePhase = EGamePhase::InGame;
//	UGameplayStatics::OpenLevel(this, FName("InGameLevel"));	// todo: Change to ServerTravel
//}
//
//void UGameFlowManager::GoToCampingPhase()
//{
//	CurrentGamePhase = EGamePhase::Camping;
//	UGameplayStatics::OpenLevel(this, FName("CampingLevel"));	// todo: Same as InGameLevel?
//}
//
//void UGameFlowManager::GoToVotingPhase()
//{
//	CurrentGamePhase = EGamePhase::Voting;
//	// No Level Change
//}
//
//void UGameFlowManager::GoToDefensePhase()
//{
//	CurrentGamePhase = EGamePhase::Defense;
//	UGameplayStatics::OpenLevel(this, FName("DefenseLevel"));	// todo: Change to ServerTravel
//}
//
//void UGameFlowManager::GoToEndingPhase()
//{
//	CurrentGamePhase = EGamePhase::Ending;
//	UGameplayStatics::OpenLevel(this, FName("EndingLevel"));	// todo: Change to ServerTravel
//}
//
//void UGameFlowManager::GoToReturnPhase()
//{
//	CurrentGamePhase = EGamePhase::ReturnToTitle;
//	// No Level Change
//}
//
