// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSystems/Datas/DataRow.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "MapDataRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ZONE064_API FMapDataRow : public FDataRow
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGamePhase GamePhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InternalMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Path;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Thumbnail;*/
};
