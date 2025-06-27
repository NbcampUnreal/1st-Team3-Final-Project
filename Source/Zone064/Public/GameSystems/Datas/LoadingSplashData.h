
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LoadingSplashData.generated.h"

USTRUCT(BlueprintType)
struct FLoadingTextRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LoadingText;
};