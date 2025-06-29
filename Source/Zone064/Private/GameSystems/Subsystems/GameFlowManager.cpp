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
				MapDataCache.FindOrAdd(Row->GamePhase).Add(*Row);
			}
		}
	}

	if (GI && GI->DestDataTable)
	{
		for (const auto& RowName : GI->DestDataTable->GetRowNames())
		{
			const FDestinationDataRow* Row = GI->DestDataTable->FindRow<FDestinationDataRow>(RowName, TEXT("DestDT"));
			if (Row)
			{
				DestDataCache.Add(RowName, *Row);
			}
		}
	}

	// Initialize GamePhase, RepeatCount
	CurGamePhaseCache = EGamePhase::None;
	CurDestinationNumCache = FName("000");
	InitCurrentRepeatCount();
}

void UGameFlowManager::RequestPhaseTransition(EGamePhase _NextGamePhase, ELevelTravelType _TravelType)
{
	const TArray<FMapDataRow>* RowArray = MapDataCache.Find(_NextGamePhase);
	if (!RowArray || RowArray->Num() == 0)
	{
		return;
	}

	// Select which map to travel
	int32 SelectedIndex = 0;
	if (_NextGamePhase == EGamePhase::InGame)
	{
		if (CurDestinationTypeCache == EMapType::Rural)
		{
			SelectedIndex = 1;
		}
	}
	const FMapDataRow& SelectedRow = (*RowArray)[SelectedIndex];

	// Cache GameFlow Data (on GameFlowManager)
	CurGamePhaseCache = _NextGamePhase;
	CurMapNameCache = SelectedRow.InternalMapName;

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
			GetWorld()->ServerTravel(SelectedRow.Path);
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

EGamePhase UGameFlowManager::GetCurGamePhaseCache()
{
	return CurGamePhaseCache;
}

int32 UGameFlowManager::GetCurRepeatCountCache()
{
	return CurRepeatCountCache;
}

FName UGameFlowManager::GetCurDestinationNumCache()
{
	return CurDestinationNumCache;
}

EMapType UGameFlowManager::GetCurDestinationTypeCache()
{
	return CurDestinationTypeCache;
}

FDestinationDataRow UGameFlowManager::GetDestDataCacheRow(FName _RowName) const
{
	const FDestinationDataRow* FoundRow = DestDataCache.Find(_RowName);
	if (FoundRow)
	{
		return *FoundRow;
	}
	return FDestinationDataRow();
}

void UGameFlowManager::SetCurDestinationNumCache(FName _DestinationNum)
{
	CurDestinationNumCache = _DestinationNum;
}

void UGameFlowManager::SetCurDestinationTypeCache(EMapType _MapType)
{
	CurDestinationTypeCache = _MapType;
}

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

