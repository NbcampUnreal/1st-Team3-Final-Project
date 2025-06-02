#include "MapGenerator/CityBlockBase.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ACityBlockBase::ACityBlockBase()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    ItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint"));
    ItemSpawnPoint->SetupAttachment(RootComponent);

    EnemySpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("EnemySpawnPoint"));
    EnemySpawnPoint->SetupAttachment(RootComponent);

    SplineBuildingSpot = CreateDefaultSubobject<USplineComponent>(TEXT("SplineBuildingSpot"));
    SplineBuildingSpot->SetupAttachment(RootComponent);

    TreeSpawnChance = 0.8f;
    LightSpawnChance = 0.3f;
    TrashSpawnChance = 0.3f;

}

void ACityBlockBase::BeginPlay()
{
    Super::BeginPlay();

    if (bIsSearchable)
    {
        if (ItemClass && FMath::RandBool())
        {
            GetWorld()->SpawnActor<AActor>(ItemClass, ItemSpawnPoint->GetComponentTransform());
        }

        if (EnemyClass && FMath::FRand() < 0.4f)
        {
            GetWorld()->SpawnActor<AActor>(EnemyClass, EnemySpawnPoint->GetComponentTransform());
        }
    }
}

//void ACityBlockBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
//    bool bFromSweep, const FHitResult& SweepResult)
//{
//    if (OtherActor && OtherActor != this)
//    {
//        OnPlayerEnterBlock();
//    }
//}

void ACityBlockBase::SetGridPosition(FIntPoint InGridPos)
{
    GridPosition = InGridPos;
}

void ACityBlockBase::OnPlayerEnterBlock()
{
    UE_LOG(LogTemp, Log, TEXT("Player entered city block: %s"), *GetName());
}

void ACityBlockBase::SpawnRoadsideProps()
{
    // 교차로면 스폰 안함 -> 나중에 수정해야함
    if (bIsCrossroad) return;

    // 격자 위치 기반 배치 제한
    switch (RoadDirection)
    {
    case ERoadDirection::Horizontal:
        if (GridPosition.X % 2 == 0) return;
        break;
    case ERoadDirection::Vertical:
        if (GridPosition.Y % 2 == 1) return;
        break;
    default:
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<USceneComponent*> Components;
    GetComponents<USceneComponent>(Components);

    for (USceneComponent* Comp : Components)
    {
        if (!Comp) continue;

        const FString Name = Comp->GetName();

        // 방향 필터
        bool bDirectionMatch = false;
        if (RoadDirection == ERoadDirection::Horizontal && (Name.Contains(TEXT("Horizontal"))))
        {
            bDirectionMatch = true;
        }
        else if (RoadDirection == ERoadDirection::Vertical && (Name.Contains(TEXT("Vertical"))))
        {
            bDirectionMatch = true;
        }

        if (!bDirectionMatch) continue;

        TArray<TSubclassOf<AActor>>* PropArray = nullptr;
        float Chance = 1.f;
        // 스폰 종류 필터
        if (Name.Contains(TEXT("Trash")))
        {
            PropArray = &TrashPrefabs;
            Chance = TrashSpawnChance;
        }
        else if (Name.Contains(TEXT("Tree")))
        {
            PropArray = &TreePrefabs;
            Chance = TreeSpawnChance;
        }
        else if (Name.Contains(TEXT("Light")))
        {
            PropArray = &LightPrefabs;
            Chance = LightSpawnChance;
        }

        if (PropArray && PropArray->Num() > 0 && GetRandomChance() < Chance)
        {
            int32 Index = FMath::RandRange(0, PropArray->Num() - 1);
            TSubclassOf<AActor> Selected = (*PropArray)[Index];

            if (Selected)
            {
                World->SpawnActor<AActor>(
                    Selected,
                    Comp->GetComponentLocation(),
                    Comp->GetComponentRotation()
                );
            }
        }
    }
}



void ACityBlockBase::InitializeBlock(FIntPoint InGridPos, bool bCrossroad, ERoadDirection InDirection)
{
    GridPosition = InGridPos;
    bIsCrossroad = bCrossroad;
    RoadDirection = InDirection;
    //SpawnRoadsideProps();
}

float ACityBlockBase::GetRandomChance()
{
    return FMath::FRand();
}

