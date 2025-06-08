// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSystems/Datas/DataRow.h"
#include "DestinationDataRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct ZONE064_API FDestinationDataRow : public FDataRow
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DestinationName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Thumbnail;
};
