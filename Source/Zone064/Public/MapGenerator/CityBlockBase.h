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
    // Ž�� ������ ������ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    bool bIsSearchable = true;

    // Ž�� Ʈ���� ����
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UBoxComponent* ExplorationTrigger;

    // ������ ���� ����Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UArrowComponent* ItemSpawnPoint;

    // �� ���� ����Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Block")
    UArrowComponent* EnemySpawnPoint;

    // ������/�� Ŭ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    TSubclassOf<AActor> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Block")
    TSubclassOf<AActor> EnemyClass;

    UFUNCTION(BlueprintCallable)
    void SetGridPosition(FIntPoint InGridPos);

    UPROPERTY(BlueprintReadOnly)
    FIntPoint GridPosition;

protected:
    // ������ �� ���� Ž�� �̺�Ʈ (Ȯ���)
    virtual void OnPlayerEnterBlock();
};
