// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MapTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EMapType : uint8
{
	None				UMETA(DisplayName = "None"),
	City				UMETA(DisplayName = "City"),
	Rural				UMETA(DisplayName = "Rural"),
};

UCLASS()
class ZONE064_API UMapTypes : public UObject
{
	GENERATED_BODY()
	
};
