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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> XLines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> YLines;
};

USTRUCT(BlueprintType)
struct FMousePositionInTile {

	GENERATED_USTRUCT_BODY()

	FMousePositionInTile() {

	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Right;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Down;
};

class ZONE064_API ZNInventoryDataStruct
{
public:

};
