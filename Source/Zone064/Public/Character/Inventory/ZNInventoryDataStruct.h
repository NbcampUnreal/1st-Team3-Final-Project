#pragma once

#include "CoreMinimal.h"
#include "ZNInventoryDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FLines {

	GENERATED_USTRUCT_BODY()

	FLines() {
		XLines = {};
		YLines = {};
	};

	TArray<FVector2D> XLines;
	TArray<FVector2D> YLines;
};

USTRUCT(BlueprintType)
struct FMousePositionInTile {

	GENERATED_USTRUCT_BODY()

	FMousePositionInTile() {

	};

	bool Right;
	bool Down;
};

class ZONE064_API ZNInventoryDataStruct
{
public:

};
