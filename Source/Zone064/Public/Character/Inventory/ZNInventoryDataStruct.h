#pragma once

#include "CoreMinimal.h"
#include "ZNInventoryDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FLines {

	// 내부적으로 GENERATED_BODY()
	GENERATED_USTRUCT_BODY()

	FLines() {
		XLines = {};
		YLines = {};
	};

	TArray<FVector2D> XLines;
	TArray<FVector2D> YLines;
};

class ZONE064_API ZNInventoryDataStruct
{
public:

};
