#include "ObjectPoolComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UObjectPoolComponent::UObjectPoolComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializePool();
}

void UObjectPoolComponent::InitializePool()
{
    if (!PooledObjectClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PooledObjectClass is not set in ObjectPoolComponent."));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    CurrentSpawnedCount = 0;
    ObjectPool.Empty(); // Clear existing pool if re-initializing

    if (bGradualInitialization && GradualSpawnRate > 0)
    {
        // Start gradual spawning
        float TimeBetweenSpawns = 1.0f / GradualSpawnRate;
        World->GetTimerManager().SetTimer(GradualSpawnTimerHandle, this, &UObjectPoolComponent::SpawnNextPooledObject, TimeBetweenSpawns, true);
    }
    else
    {
        // Spawn all at once (original behavior)
        for (int32 i = 0; i < PoolSize; ++i)
        {
            SpawnSinglePooledObject();
        }
    }
}

void UObjectPoolComponent::SpawnNextPooledObject()
{
    if (CurrentSpawnedCount < PoolSize)
    {
        SpawnSinglePooledObject();
        CurrentSpawnedCount++;
    }
    else
    {
        // All objects spawned, clear timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(GradualSpawnTimerHandle);
        }
    }
}

void UObjectPoolComponent::SpawnSinglePooledObject()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AActor* PooledObject = World->SpawnActor<AActor>(PooledObjectClass, FVector::ZeroVector, FRotator::ZeroRotator);
    if (PooledObject)
    {
        PooledObject->SetActorHiddenInGame(true);
        PooledObject->SetActorEnableCollision(false);
        PooledObject->SetActorTickEnabled(false);

        IPoolable* Poolable = Cast<IPoolable>(PooledObject);
        if (Poolable)
        {
            Poolable->Execute_OnPoolEnd(PooledObject);
        }

        ObjectPool.Add(PooledObject);
    }
}


AActor* UObjectPoolComponent::SpawnPooledObject(const FTransform& SpawnTransform)
{
    if (ObjectPool.Num() > 0)
    {
        AActor* PooledObject = ObjectPool.Pop();
        if (PooledObject)
        {
            PooledObject->SetActorTransform(SpawnTransform);
            PooledObject->SetActorHiddenInGame(false);
            PooledObject->SetActorEnableCollision(true);
            PooledObject->SetActorTickEnabled(true);

            IPoolable* Poolable = Cast<IPoolable>(PooledObject);
            if (Poolable)
            {
                Poolable->Execute_OnPoolBegin(PooledObject);
            }

            return PooledObject;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No available objects in pool. Consider increasing pool size."));
    return nullptr;
}

void UObjectPoolComponent::ReturnPooledObject(AActor* ActorToReturn)
{
    if (ActorToReturn)
    {
        IPoolable* Poolable = Cast<IPoolable>(ActorToReturn);
        if (Poolable)
        {
            Poolable->Execute_OnPoolEnd(ActorToReturn);
        }

        ActorToReturn->SetActorHiddenInGame(true);
        ActorToReturn->SetActorEnableCollision(false);
        ActorToReturn->SetActorTickEnabled(false);
        ObjectPool.Add(ActorToReturn);
    }
}
