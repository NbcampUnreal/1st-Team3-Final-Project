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
	None					UMETA(DisplayName = "None"),				// 게임 시작 직후, 또는 에러 상황
	Boot					UMETA(DisplayName = "Boot"),				// 게임 시작 화면
	Menu					UMETA(DisplayName = "Menu"),				// 세션 생성/참가 화면 (Steam 연동)
	Lobby					UMETA(DisplayName = "Lobby"),				// 세션 대기방 (플레이어 준비, 게임 진입)
	Departure_Day			UMETA(DisplayName = "Departure_Day"),		// 차량 이동 오전 연출 (시네마틱?)				-> 4회 반복 시작지점
	Departure_Event			UMETA(DisplayName = "Departure_Event"),		// 차량 이동 중 일반이벤트 발생!
	InGame					UMETA(DisplayName = "InGame"),				// 전투 지역
	Departure_Night			UMETA(DisplayName = "Departure_Night"),		// 차량 이동 오후 연출 (시네마틱?)
	Camping					UMETA(DisplayName = "Camping"),				// 정비 시간 (크래프팅 등)
	Camping_Event			UMETA(DisplayName = "Camping_Event"),		// 정비 시간 중 일반이벤트 발생!				-> 4회 반복 종료지점
	Special_Event			UMETA(DisplayName = "Special_Event"),		// 특수이벤트 발생!
	Defense					UMETA(DisplayName = "Defense"),				// 디펜스 지역
	Ending					UMETA(DisplayName = "Ending"),				// 엔딩 연출 (크레딧)
	Test					UMETA(DisplayName = "Test"),				// 테스트용 (템플릿 전용 레벨?)
};

UCLASS()
class ZONE064_API UGamePhaseTypes : public UObject
{
	GENERATED_BODY()
	
};
