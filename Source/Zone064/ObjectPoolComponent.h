#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Poolable.h"
#include "ObjectPoolComponent.generated.h"

// Forward declaration
class AActor;

USTRUCT(BlueprintType)
struct FObjectPool
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AActor*> AvailableActors;

    UPROPERTY()
    TArray<AActor*> InUseActors;
};

USTRUCT(BlueprintType)
struct FPooledActorInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    int32 InitialSize = 20; // Default value
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZONE064_API UObjectPoolComponent : public UActorComponent
{
    GENERATED_BODY()

    friend class AWaveManager;

public:
    UObjectPoolComponent();

    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void InitializePool();

    UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (DisplayName = "Spawn Pooled Object By Class"))
    AActor* SpawnPooledObject(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void ReturnPooledObject(AActor* ActorToReturn);

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
    TArray<FPooledActorInfo> PooledActorInfos;

private:

    UPROPERTY()
    TMap<TSubclassOf<AActor>, FObjectPool> PoolMap;
    void CreateAndPoolObject(TSubclassOf<AActor> ActorClass);
};
