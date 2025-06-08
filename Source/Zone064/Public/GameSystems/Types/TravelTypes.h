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
	NoTravel				UMETA(DisplayName = "NoTravel"),				// 레벨 전환 없이 페이즈 전환만 필요한 경우
	OpenLevel_Solo			UMETA(DisplayName = "OpenLevel_Solo"),			// 
	OpenLevel_Listen		UMETA(DisplayName = "OpenLevel_Listen"),		// 
	ServerTravel			UMETA(DisplayName = "ServerTravel"),			// 
};

UCLASS()
class ZONE064_API UTravelTypes : public UObject
{
	GENERATED_BODY()
	
};
