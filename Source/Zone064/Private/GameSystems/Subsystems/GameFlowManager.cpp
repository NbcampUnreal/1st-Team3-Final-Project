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

	// Initialize GamePhase, Map
	CurrentGamePhase = EGamePhase::None;
	//AdvanceGamePhase();
}

//void UGameFlowManager::AdvanceGamePhase()
//{
//	switch (CurrentGamePhase)
//	{
//	case EGamePhase::None:
//	{
//		InitCurrentRepeatCount();
//		ChangePhaseAndMap(EGamePhase::Title);
//		break;
//	}
//	case EGamePhase::Title:
//	{
//		ChangePhaseAndMap(EGamePhase::Menu);
//		break;
//	}
//	case EGamePhase::Menu:
//	{
//		ChangePhaseAndMap(EGamePhase::Lobby);
//		break;
//	}
//	case EGamePhase::Lobby:
//	{
//		ChangePhaseAndMap(EGamePhase::Departure);
//		break;
//	}
//	case EGamePhase::Departure:
//	{
//		ChangePhaseAndMap(EGamePhase::Driving);
//		break;
//	}
//	case EGamePhase::Driving:
//	{
//		ChangePhaseAndMap(EGamePhase::InGame);
//		break;
//	}
//	case EGamePhase::InGame:
//	{
//		ChangePhaseAndMap(EGamePhase::Camping);
//		break;
//	}
//	case EGamePhase::Camping:
//	{
//		ChangePhaseAndMap(EGamePhase::Voting);
//		break;
//	}
//	case EGamePhase::Voting:
//	{
//		if (CurrentRepeatCount < MaxRepeatCount)
//		{
//			AddCurrentRepeatCount();
//			ChangePhaseAndMap(EGamePhase::Driving);	// next loop
//		}
//		else
//		{
//			ChangePhaseAndMap(EGamePhase::Defense);	// end loop
//		}
//		break;
//	}
//	case EGamePhase::Defense:
//	{
//		ChangePhaseAndMap(EGamePhase::Ending);
//		break;
//	}
//	case EGamePhase::Ending:
//	{
//		ChangePhaseAndMap(EGamePhase::ReturnToTitle);
//		break;
//	}
//	case EGamePhase::ReturnToTitle:
//	{
//		InitCurrentRepeatCount();
//		ChangePhaseAndMap(EGamePhase::Title);
//		break;
//	}
//	default:
//	{
//		break;
//	}
//	}
//}

void UGameFlowManager::ChangeGamePhase(EGamePhase _NextGamePhase)
{
	SetCurrentGamePhase(_NextGamePhase);
}

void UGameFlowManager::ChangeMapByName(FName _NextMapName, bool _bServerTravel)
{
	if (_NextMapName != CurrentMapName)
	{
		SetCurrentMapName(_NextMapName);

		if (_bServerTravel)
		{
			if (GetWorld()->GetAuthGameMode())
			{
				FString TravelPath = FString::Printf(TEXT("/Game/Maps/Test/%s"), *_NextMapName.ToString());
				GetWorld()->ServerTravel(TravelPath);
			}
		}
		else
		{
			UGameplayStatics::OpenLevel(this, _NextMapName);
		}
	}
}

void UGameFlowManager::ChangeMapByPhase(EGamePhase _NextGamePhase, bool _bServerTravel)
{
	/*FName NextMapName = GetInternalMapNameByPhase(_NextGamePhase);
	ChangeMapByName(NextMapName, _bServerTravel);*/
}

void UGameFlowManager::ChangePhaseAndMap(EGamePhase _NextGamePhase, bool _bServerTravel)
{
	/*ChangeGamePhase(_NextGamePhase);

	FName NextMapName = GetInternalMapNameByPhase(_NextGamePhase);
	ChangeMapByName(NextMapName, _bServerTravel);*/
}

void UGameFlowManager::RequestPhaseTransition(EGamePhase _NextGamePhase, ELevelTravelType _TravelType)
{
	const FMapDataRow* Row = MapDataCache.Find(_NextGamePhase);
	if (!Row)
	{
		return;
	}

	SetCurrentGamePhase(_NextGamePhase);
	SetCurrentMapName(Row->InternalMapName);
	
	switch (_TravelType)
	{
	case ELevelTravelType::NoTravel:
	{
		break;
	}
	case ELevelTravelType::OpenLevel_Solo:
	{
		UGameplayStatics::OpenLevel(this, Row->InternalMapName);
		break;
	}

	case ELevelTravelType::OpenLevel_Listen:
	{
		UGameplayStatics::OpenLevel(this, Row->InternalMapName, true, TEXT("listen"));
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


	//OnPhaseChanged.Broadcast(NextPhase);
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
	CurrentRepeatCount = 1;
}

void UGameFlowManager::AddCurrentRepeatCount()
{
	CurrentRepeatCount += 1;
}

