// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSystems/Subsystems/UIManager.h"
#include "GameSystems/Datas/WidgetDataRow.h"

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	//static const FString Context = TEXT("WidgetDT");
	//for (const auto& RowName : WidgetDataTable->GetRowNames())
	//{
	//	const FWidgetDataRow* Row = WidgetDataTable->FindRow<FWidgetDataRow>(RowName, Context);
	//	if (!Row)
	//	{
	//		continue;
	//	}

	//	if (Row->WidgetClass)
	//	{
	//		//WidgetCache.Add(Row->Phase, Row->WidgetClass);
	//	}
	//}

	// todo: 
	// - datatable 안의 항목 결정되면 캐싱파트 작성
	// - GameFlowManager 참고하여 GI에서 끌어오는 방식 사용
	// - 전부 일단 화면에 올린 뒤에 비활성화할건지? 아니면 전부 해제한 뒤에 특정 레벨에 필요한 녀석들만 올리기???
}
