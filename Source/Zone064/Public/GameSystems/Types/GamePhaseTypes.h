// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GamePhaseTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None				UMETA(DisplayName = "Title"),				// 게임 시작 직후, 또는 에러 상황
	Title				UMETA(DisplayName = "Title"),				// 게임 시작 화면
	Menu				UMETA(DisplayName = "Menu"),				// 세션 생성/참가 화면 (Steam 연동)
	Lobby				UMETA(DisplayName = "Lobby"),				// 세션 대기방 (플레이어 준비, 게임 진입)
	Departure			UMETA(DisplayName = "Departure"),			// 출발 준비 연출 (시네마틱 등)
	Driving				UMETA(DisplayName = "Driving"),				// 목적지 이동 (이벤트 발생!)			-> 4회 반복 시작지점
	InGame				UMETA(DisplayName = "InGame"),				// 전투, 파밍
	Camping				UMETA(DisplayName = "Camping"),				// 정비 시간 (이벤트 발생!, 크래프팅 등)
	Voting				UMETA(DisplayName = "Voting"),				// 다음 이동 지역 선택				-> 4회 반복 종료지점
	Defense				UMETA(DisplayName = "Defense"),				// 최종 디펜스
	Ending				UMETA(DisplayName = "Ending"),				// 엔딩 연출, 게임 결과 집계
	ReturnToTitle		UMETA(DisplayName = "ReturnToTitle"),		// 세션 파괴, Title로 돌아가기
};

UCLASS()
class ZONE064_API UGamePhaseTypes : public UObject
{
	GENERATED_BODY()
	
};
