#include "MapGenerator/MapGenerator.h"
#include "MapGenerator/CityBlockBase.h"
#include "Kismet/KismetMathLibrary.h"

AMapGenerator::AMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMapGenerator::BeginPlay()
{
    Super::BeginPlay();

    // 모든 프리팹 soft ref 로드 요청
    TArray<FSoftObjectPath> AssetPaths;
    for (const auto& Prefab : BlockPrefabAssets)
    {
        AssetPaths.Add(Prefab.ToSoftObjectPath());
    }

    AssetLoader.RequestAsyncLoad(AssetPaths, FStreamableDelegate::CreateUObject(this, &AMapGenerator::OnPrefabsLoaded));

}

void AMapGenerator::OnPrefabsLoaded()
{
    // 실제 클래스 캐싱
    for (const auto& SoftClass : BlockPrefabAssets)
    {
        if (SoftClass.IsValid())
        {
            CachedPrefabs.Add(SoftClass.Get());
        }
    }

    // 맵 생성 가능
    GenerateMap();
}

void AMapGenerator::GenerateMap()
{
    if (CachedPrefabs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapGenerator: CachedPrefabs is empty!"));
        return;
    }

    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            int32 Index = RandomStream.RandRange(0, CachedPrefabs.Num() - 1);
            TSubclassOf<AActor> SelectedPrefab = CachedPrefabs[Index];

            if (!SelectedPrefab) continue;

            FVector Location = GetActorLocation() + FVector(X * TileSize, Y * TileSize, 0.0f);

            // 랜덤 방향 설정
            int32 DirectionIndex = RandomStream.RandRange(0, 3);
            float YawRotation = DirectionIndex * 90.0f;
            FRotator Rotation = FRotator(0.0f, YawRotation, 0.0f);

            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SelectedPrefab, Location, Rotation);
            if (SpawnedActor)
            {
                if (ACityBlockBase* Block = Cast<ACityBlockBase>(SpawnedActor))
                {
                    Block->SetGridPosition(FIntPoint(X, Y));
                }

                UE_LOG(LogTemp, Log, TEXT("Spawned: %s at Grid (%d, %d), Rotation: %.0f"), *SpawnedActor->GetName(), X, Y, YawRotation);
            }
        }
    }
}



