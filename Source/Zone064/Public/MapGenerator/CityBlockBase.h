#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

public:
    // 탐색 가능한 블럭인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    bool bIsSearchable = true;

    // 탐색 트리거 영역
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UBoxComponent* ExplorationTrigger;

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

    UFUNCTION(BlueprintCallable)
    void SetGridPosition(FIntPoint InGridPos);

    UPROPERTY(BlueprintReadOnly)
    FIntPoint GridPosition;

protected:
    // 오버랩 시 실제 탐색 이벤트 (확장용)
    virtual void OnPlayerEnterBlock();
};
