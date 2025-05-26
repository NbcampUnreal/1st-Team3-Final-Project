// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSystems/Datas/DataRow.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "WidgetDataRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ZONE064_API FWidgetDataRow : public FDataRow
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGamePhase GamePhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> WidgetClass;
};

