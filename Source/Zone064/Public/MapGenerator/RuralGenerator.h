// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator/MapGenerator.h"
#include "RuralGenerator.generated.h"

UCLASS()
class ZONE064_API ARuralGenerator : public AMapGenerator
{
	GENERATED_BODY()
	
public:	
	ARuralGenerator();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxShopCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHouseCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxFiveByFiveShopCount;

	bool bShopLimitReached;
	bool bHouseLimitReached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AlleyLightSpawnChance;

	// 좌우 방향 2칸 이내 탐색용 방향 오프셋
	TArray<FIntPoint> FurtherLeftRightOffsetList;
	// 상하 방향 탐색용 방향 오프셋
	TArray<FIntPoint> UpDownOffsetList;

public:	
	
	virtual void GenerateZoneMap() override;

	// 골목길 가로등 스폰용
	void TrySpawnBorder(const FVector& Location, const FIntPoint& GridPos) override;

};
