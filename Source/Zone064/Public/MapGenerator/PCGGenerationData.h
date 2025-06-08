#pragma once

#include "CoreMinimal.h"
#include "PCGGenerationData.generated.h"

USTRUCT(BlueprintType)
struct ZONE064_API FPCGGenerationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RandomSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Floor;

    FPCGGenerationData()
        : RandomSeed(0)
        , Floor(3)
    {
    }
};
