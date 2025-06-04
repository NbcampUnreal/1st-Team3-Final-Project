#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "MapGenerator/MapGenerator.h"
#include "CityBlockBase.generated.h"

class UBoxComponent;
class UArrowComponent;

UCLASS()
class ZONE064_API ACityBlockBase : public AActor
{
    GENERATED_BODY()

public:
    ACityBlockBase();

protected:
    virtual void BeginPlay() override;

    //UFUNCTION()
    //void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    //    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    //    bool bFromSweep, const FHitResult& SweepResult);

public:
    // 탐색 가능한 블럭인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    bool bIsSearchable = true;

    // 아이템 스폰 포인트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UArrowComponent* ItemSpawnPoint;

    // 적 스폰 포인트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UArrowComponent* EnemySpawnPoint;

    // 아이템/적 클래스 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    TSubclassOf<AActor> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    TSubclassOf<AActor> EnemyClass;

    // Spawn Random Building
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    USplineComponent* SplineBuildingSpot;

    // 도로 소품 프리팹 
    UPROPERTY(EditAnywhere, Category = "Props")
    TArray<TSubclassOf<AActor>> TrashPrefabs;
    UPROPERTY(EditAnywhere, Category = "Props")
    TArray<TSubclassOf<AActor>> TreePrefabs;
    UPROPERTY(EditAnywhere, Category = "Props")
    TArray<TSubclassOf<AActor>> LightPrefabs;

    // 소품 스폰 지점
    UPROPERTY(EditAnywhere, Category = "Props|Points")
    TArray<USceneComponent*> TreePoints;

    UPROPERTY(EditAnywhere, Category = "Props|Points")
    TArray<USceneComponent*> LightPoints;

    UPROPERTY(EditAnywhere, Category = "Props|Points")
    TArray<USceneComponent*> TrashPoints;

    UPROPERTY(EditAnywhere, Category = "Props|Chance")
    float TreeSpawnChance;

    UPROPERTY(EditAnywhere, Category = "Props|Chance")
    float LightSpawnChance;

    UPROPERTY(EditAnywhere, Category = "Props|Chance")
    float TrashSpawnChance;

    UFUNCTION(BlueprintCallable)
    void SetGridPosition(FIntPoint InGridPos);

    UPROPERTY(BlueprintReadOnly)
    FIntPoint GridPosition;

    UFUNCTION(BlueprintCallable)
    void InitializeBlock(FIntPoint InGridPos, bool bCrossroad, ERoadDirection InDirection);

    float GetRandomChance();

    UPROPERTY(BlueprintReadOnly, Category = "City Block")
    bool bIsCrossroad;

protected:
    // 오버랩 시 실제 탐색 이벤트 (확장용)
    virtual void OnPlayerEnterBlock();

    void SpawnRoadsideProps();



    ERoadDirection RoadDirection;

};
