// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "WaveManager.generated.h"

USTRUCT(BlueprintType)
struct FWaveSpawnerGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager|WaveSpawnerGroup")
	FName TagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager|WaveSpawnerGroup")
	float SpawnInterval;
};

USTRUCT(BlueprintType)
struct FWaveInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager|WaveInfo")
	FName TagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager|WaveInfo")
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager|WaveInfo")
	float SpawnChance;
};

UCLASS()
class ZONE064_API AWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveManager();
	void GetAllTaggedActors();
	void ActivateSpawners(FName TagName);
	void SpawnWave(FName TagName);
	
	// AI 명시적 시작, 플레이어 핑
	void GetAIControllerAndStartLogic(AActor* SpawnedActor);
	void NotifyRandomPlayerLocation();

protected:
	virtual void BeginPlay() override;

	TMap<FName, FTimerHandle> WaveTimerHandles;
	TMap<FName, TArray<TObjectPtr<AActor>>> SpawnerMap;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager")
	TArray<FWaveSpawnerGroup> SpawnerGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager")
	TArray<FWaveInfo> WaveInfos;

	
};
