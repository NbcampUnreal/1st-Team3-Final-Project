#include "MapGenerator/CityBlockBase.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"

ACityBlockBase::ACityBlockBase()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    ExplorationTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExplorationTrigger"));
    ExplorationTrigger->SetupAttachment(RootComponent);
    ExplorationTrigger->SetCollisionProfileName(TEXT("Trigger"));
    ExplorationTrigger->SetBoxExtent(FVector(300.f, 300.f, 200.f));
    ExplorationTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACityBlockBase::OnOverlapBegin);

    ItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("ItemSpawnPoint"));
    ItemSpawnPoint->SetupAttachment(RootComponent);

    EnemySpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("EnemySpawnPoint"));
    EnemySpawnPoint->SetupAttachment(RootComponent);

    SplineBuildingSpot = CreateDefaultSubobject<USplineComponent>(TEXT("SplineBuildingSpot"));
    SplineBuildingSpot->SetupAttachment(RootComponent);
}

void ACityBlockBase::BeginPlay()
{
    Super::BeginPlay();

    // �ʱ� Ž�� ���� ���ο� ���� ����
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

void ACityBlockBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // �÷��̾� ���� (�÷��̾� üũ�� ������Ʈ�� �°� ����)
    if (OtherActor && OtherActor != this)
    {
        OnPlayerEnterBlock();
    }
}

void ACityBlockBase::SetGridPosition(FIntPoint InGridPos)
{
    GridPosition = InGridPos;
}

void ACityBlockBase::OnPlayerEnterBlock()
{
    UE_LOG(LogTemp, Log, TEXT("Player entered city block: %s"), *GetName());
    // ���⼭ Ž�� �̺�Ʈ, UI �˸�, ����Ʈ Ʈ���� �� ���� ����
}
