#include "ObjectPoolComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActorComponent/AILODComponent.h"
#include "GameFramework/Character.h"

UObjectPoolComponent::UObjectPoolComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner()->HasAuthority())
    {
        InitializePool();
    }
}

void UObjectPoolComponent::InitializePool()
{
    if (PooledActorInfos.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No PooledActorInfos set in ObjectPoolComponent."));
        return;
    }

    for (const FPooledActorInfo& ActorInfo : PooledActorInfos)
    {
        if (!ActorInfo.ActorClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("Null ActorClass found in PooledActorInfos array."));
            continue;
        }

        FObjectPool& Pool = PoolMap.FindOrAdd(ActorInfo.ActorClass);
        for (int32 i = 0; i < ActorInfo.InitialSize; ++i)
        {
            CreateAndPoolObject(ActorInfo.ActorClass);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("InitializePool Called for %d classes on %s"), PooledActorInfos.Num(), *GetOwner()->GetName());
}

void UObjectPoolComponent::CreateAndPoolObject(TSubclassOf<AActor> ActorClass)
{
    UWorld* World = GetWorld();
    if (!World || !ActorClass) return;

    AActor* PooledObject = World->SpawnActor<AActor>(ActorClass, FVector(0.f, 0.f, -999.f), FRotator::ZeroRotator);
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

        if (GetOwner()->HasAuthority())
        {
            if (UCharacterMovementComponent* MovementComponent = PooledObject->FindComponentByClass<UCharacterMovementComponent>())
            {
                MovementComponent->SetMovementMode(EMovementMode::MOVE_None);
            }
        }

        FObjectPool& Pool = PoolMap.FindOrAdd(ActorClass);
        Pool.AvailableActors.Add(PooledObject);
    }
}

AActor* UObjectPoolComponent::SpawnPooledObject(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnPooledObject called with null ActorClass."));
        return nullptr;
    }

    FObjectPool* Pool = PoolMap.Find(ActorClass);
    if (!Pool)
    {        
        UE_LOG(LogTemp, Warning, TEXT("No pool found for class %s. Make sure it's in PooledActorInfos and initialized."), *ActorClass->GetName());
        return nullptr;
    }

    AActor* PooledObject = nullptr;
    if (Pool->AvailableActors.Num() > 0)
    {
        PooledObject = Pool->AvailableActors.Pop();
    }
    else
    {
        // Dynamically expand the pool if no actors are available
        UE_LOG(LogTemp, Log, TEXT("No available objects for %s, expanding pool size."), *ActorClass->GetName());
        CreateAndPoolObject(ActorClass);
        if (Pool->AvailableActors.Num() > 0)
        {
            PooledObject = Pool->AvailableActors.Pop();
        }
    }

    if (PooledObject)
    {
        Pool->InUseActors.Add(PooledObject);

        PooledObject->SetActorTransform(SpawnTransform);
        PooledObject->SetActorHiddenInGame(false);
        PooledObject->SetActorEnableCollision(true);
        PooledObject->SetActorTickEnabled(true);

        ACharacter* Character = Cast<ACharacter>(PooledObject);
        if (Character && GetOwner()->HasAuthority())
        {
            if (!Character->GetController())
            {
                Character->SpawnDefaultController();
            }
        }

        IPoolable* Poolable = Cast<IPoolable>(PooledObject);
        if (Poolable)
        {
            Poolable->Execute_OnPoolBegin(PooledObject);
        }

        if (GetOwner()->HasAuthority())
        {
            if (UAILODComponent* LODComponent = PooledObject->FindComponentByClass<UAILODComponent>())
            {
                LODComponent->Deactivate();
                LODComponent->SetIsPooled(false);
            }

            if (UCharacterMovementComponent* MovementComponent = PooledObject->FindComponentByClass<UCharacterMovementComponent>())
            {
                MovementComponent->ResetMoveState();
                MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
            }
        }

        return PooledObject;
    }

    UE_LOG(LogTemp, Warning, TEXT("Failed to spawn an object for class %s."), *ActorClass->GetName());
    return nullptr;
}

void UObjectPoolComponent::ReturnPooledObject(AActor* ActorToReturn)
{
    if (!ActorToReturn) return;

    TSubclassOf<AActor> ActorClass = ActorToReturn->GetClass();
    FObjectPool* Pool = PoolMap.Find(ActorClass);

    if (Pool && Pool->InUseActors.Contains(ActorToReturn))
    {
        Pool->InUseActors.Remove(ActorToReturn);
        Pool->AvailableActors.Add(ActorToReturn);

        if (UAILODComponent* LODComponent = ActorToReturn->FindComponentByClass<UAILODComponent>())
        {
            LODComponent->SetIsPooled(true);
        }

        IPoolable* Poolable = Cast<IPoolable>(ActorToReturn);
        if (Poolable)
        {
            Poolable->Execute_OnPoolEnd(ActorToReturn);
        }

        ActorToReturn->SetActorHiddenInGame(true);
        ActorToReturn->SetActorEnableCollision(false);
        ActorToReturn->SetActorTickEnabled(false);

        if (GetOwner()->HasAuthority())
        {
            if (UCharacterMovementComponent* MovementComponent = ActorToReturn->FindComponentByClass<UCharacterMovementComponent>())
            {
                MovementComponent->SetMovementMode(EMovementMode::MOVE_None);
            }
        }
    }
    else
    {
        // This actor doesn't belong to this pool or was already returned.
        // You might want to destroy it or log a warning.
        UE_LOG(LogTemp, Warning, TEXT("Returned actor %s does not belong to any managed pool or was already returned. Destroying it."), *ActorToReturn->GetName());
        ActorToReturn->Destroy();
    }
}
