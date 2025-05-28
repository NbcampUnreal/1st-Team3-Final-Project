// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystems/Subsystems/UIManager.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "GameSystems/Datas/WidgetDataRow.h"

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Cache Data Table
	UZNBaseGameInstance* GI = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (GI && GI->MapNameDataTable)
	{
		//static const FString Context = TEXT("WidgetDT");
		//for (const auto& RowName : GI->WidgetDataTable->GetRowNames())
		//{
		//	const FWidgetDataRow* Row = GI->WidgetDataTable->FindRow<FWidgetDataRow>(RowName, Context);
		//	if (!Row)
		//	{
		//		continue;
		//	}

		//	if (Row->WidgetClass)
		//	{
		//		//WidgetCache.Add(Row->Phase, Row->WidgetClass);
		//	}
		//}
	}

	// todo: 
	// - datatable 안의 항목 캐싱파트 작성
	// - GameFlowManager 참고하여 GI에서 끌어오는 방식 사용
}
