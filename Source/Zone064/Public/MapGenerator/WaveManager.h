// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "WaveManager.generated.h"

class UObjectPoolComponent;
struct FObjectPool;

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

	// 웨이브 시작
	UFUNCTION(BlueprintCallable, Category = "WaveManager")
	void StartWave(FName TagName, int32 NumberToSpawn, float PreparationTime);
	void MoveWave();

	// AI 명시적 시작, 랜덤 플레이어
	void GetAIControllerAndStartLogic(AActor* SpawnedActor);
	APawn* GetRandomPlayerPawn();

protected:
	virtual void BeginPlay() override;

	void ExecuteWaveSpawn(FName TagName, int32 NumberToSpawn);

	TMap<FName, FTimerHandle> WaveTimerHandles;
	TMap<FName, TArray<TObjectPtr<AActor>>> SpawnerMap;
	
	// 오브젝트풀 컴포넌트와 구조체
	UObjectPoolComponent* ObjectPoolComponent;
	FObjectPool* ObjectPool;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager")
	float SpawnSpreadRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager")
	TArray<FWaveSpawnerGroup> SpawnerGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveManager")
	TArray<FWaveInfo> WaveInfos;

	
};
