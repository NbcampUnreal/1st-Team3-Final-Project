// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TravelTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ELevelTravelType : uint8
{
	NoTravel				UMETA(DisplayName = "NoTravel"),				// 레벨 이동 없이 Phase만 변경
	OpenLevel_Solo			UMETA(DisplayName = "OpenLevel_Solo"),			// 세션을 열지 않고 단독으로 레벨 이동, Phase 변경
	OpenLevel_Listen		UMETA(DisplayName = "OpenLevel_Listen"),		// 세션을 열며 단독으로 레벨 이동, Phase 변경
	ServerTravel			UMETA(DisplayName = "ServerTravel"),			// 서버가 전체 플레이어를 한꺼번에 레벨 이동, Phase 변경
};

UCLASS()
class ZONE064_API UTravelTypes : public UObject
{
	GENERATED_BODY()
	
};
