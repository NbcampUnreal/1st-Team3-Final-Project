// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystems/Subsystems/GameFlowManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "GameStates/ZNBaseGameState.h"
#include "GameSystems/Datas/MapDataRow.h"


void UGameFlowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache Data Table
	UZNBaseGameInstance* GI = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (GI && GI->MapDataTable)
	{
		for (const auto& RowName : GI->MapDataTable->GetRowNames())
		{
			const FMapDataRow* Row = GI->MapDataTable->FindRow<FMapDataRow>(RowName, TEXT("MapDT"));
			if (Row)
			{
				MapDataCache.Add(Row->GamePhase, *Row);
			}
		}
	}

	// Initialize GamePhase, RepeatCount
	CurGamePhaseCache = EGamePhase::None;
	InitCurrentRepeatCount();
}

//void UGameFlowManager::ChangeGamePhase(EGamePhase _NextGamePhase)
//{
//	SetCurrentGamePhase(_NextGamePhase);
//}
//
//void UGameFlowManager::ChangeMapByName(FName _NextMapName, bool _bServerTravel)
//{
//	if (_NextMapName != CurrentMapName)
//	{
//		SetCurrentMapName(_NextMapName);
//
//		if (_bServerTravel)
//		{
//			if (GetWorld()->GetAuthGameMode())
//			{
//				FString TravelPath = FString::Printf(TEXT("/Game/Maps/Test/%s"), *_NextMapName.ToString());
//				GetWorld()->ServerTravel(TravelPath);
//			}
//		}
//		else
//		{
//			UGameplayStatics::OpenLevel(this, _NextMapName);
//		}
//	}
//}
//
//void UGameFlowManager::ChangeMapByPhase(EGamePhase _NextGamePhase, bool _bServerTravel)
//{
//	/*FName NextMapName = GetInternalMapNameByPhase(_NextGamePhase);
//	ChangeMapByName(NextMapName, _bServerTravel);*/
//}
//
//void UGameFlowManager::ChangePhaseAndMap(EGamePhase _NextGamePhase, bool _bServerTravel)
//{
//	/*ChangeGamePhase(_NextGamePhase);
//
//	FName NextMapName = GetInternalMapNameByPhase(_NextGamePhase);
//	ChangeMapByName(NextMapName, _bServerTravel);*/
//}

void UGameFlowManager::RequestPhaseTransition(EGamePhase _NextGamePhase, ELevelTravelType _TravelType)
{
	const FMapDataRow* Row = MapDataCache.Find(_NextGamePhase);
	if (!Row)
	{
		return;
	}

	// Cache GameFlow Data (on GameFlowManager)
	CurGamePhaseCache = _NextGamePhase;
	CurMapNameCache = Row->InternalMapName;

	// Level Travel
	switch (_TravelType)
	{
	case ELevelTravelType::NoTravel:
	{
		break;
	}
	case ELevelTravelType::OpenLevel_Solo:
	{
		UGameplayStatics::OpenLevel(this, CurMapNameCache);
		break;
	}

	case ELevelTravelType::OpenLevel_Listen:
	{
		UGameplayStatics::OpenLevel(this, CurMapNameCache, true, TEXT("listen"));
		break;
	}
	case ELevelTravelType::ServerTravel:
	{
		if (GetWorld()->GetAuthGameMode())
		{
			GetWorld()->ServerTravel(Row->Path);
		}
		break;
	}
	default:
		break;
	}

	// Update GameFlow Data (from GameFlowManager to GameState)
	if (GetWorld()->GetAuthGameMode())
	{
		UpdateGameFlowData();
	}


	//OnPhaseChanged.Broadcast(NextPhase);
}

void UGameFlowManager::UpdateGameFlowData()
{
	AZNBaseGameState* GS = GetWorld()->GetGameState<AZNBaseGameState>();
	if (GS)
	{
		GS->SetCurrentGamePhase(CurGamePhaseCache);
		GS->SetCurrentMapName(CurMapNameCache);
		GS->SetCurrentRepeatCount(CurRepeatCountCache);
	}
}

//FName UGameFlowManager::GetInternalMapNameByPhase(EGamePhase _GamePhase)
//{
//	const FMapDataRow* Row = MapDataCache.Find(_GamePhase);
//	if (!Row)
//	{
//		return;
//	}
//
//	FName InternalMapName = Row->InternalMapName;
//
//	return InternalMapName;
//}

void UGameFlowManager::InitCurrentRepeatCount()
{
	CurRepeatCountCache = 0;
}

void UGameFlowManager::AddCurrentRepeatCount()
{
	CurRepeatCountCache += 1;
}

bool UGameFlowManager::CheckMaxRepeatCount()
{
	if (CurRepeatCountCache < MaxRepeatCount)
	{
		return false;
	}

	return true;
}

