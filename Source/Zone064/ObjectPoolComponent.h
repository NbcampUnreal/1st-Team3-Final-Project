#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Poolable.h"
#include "ObjectPoolComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZONE064_API UObjectPoolComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UObjectPoolComponent();

    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void InitializePool();

    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    AActor* SpawnPooledObject(const FTransform& SpawnTransform);

    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void ReturnPooledObject(AActor* ActorToReturn);

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    TSubclassOf<AActor> PooledObjectClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    int32 PoolSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    bool bGradualInitialization = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool", meta = (EditCondition = "bGradualInitialization", ClampMin = "0.1"))
    float GradualSpawnRate = 10.0f; // Actors per second

private:
    UPROPERTY()
    TArray<AActor*> ObjectPool;

    FTimerHandle GradualSpawnTimerHandle;
    int32 CurrentSpawnedCount;
    void SpawnSinglePooledObject();
    void SpawnNextPooledObject();
};
