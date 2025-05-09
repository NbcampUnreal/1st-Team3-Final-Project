#include "MapGenerator/MapGenerator.h"
#include "MapGenerator/CityBlockBase.h"
#include "Kismet/KismetMathLibrary.h"

AMapGenerator::AMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    // 시드
    Seed = 42;
    // 특수부지 최소 크기와 확률
    RequiredClusterSize = 6;
    SpecialChance = 0.5f;
}

void AMapGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (Seed < 0)
    {
        Seed = FDateTime::Now().ToUnixTimestamp();
    }
    RandomStream.Initialize(Seed);

    // BlockPrefabSets -> BlockPrefabAssetsByZone 변환
    for (const FZonePrefabSet& Set : BlockPrefabSets)
    {
        BlockPrefabAssetsByZone.FindOrAdd(Set.ZoneType) = Set.Prefabs;
    }

    // 소프트 레퍼런스 로딩
    TArray<FSoftObjectPath> AssetPaths;
    for (const auto& Pair : BlockPrefabAssetsByZone)
    {
        for (const auto& Prefab : Pair.Value)
        {
            AssetPaths.Add(Prefab.ToSoftObjectPath());
        }
    }

    AssetLoader.RequestAsyncLoad(AssetPaths, FStreamableDelegate::CreateUObject(this, &AMapGenerator::OnPrefabsLoaded));
}



void AMapGenerator::OnPrefabsLoaded()
{
    for (const auto& Pair : BlockPrefabAssetsByZone)
    {
        EZoneType Zone = Pair.Key;

        for (const auto& SoftClass : Pair.Value)
        {
            if (SoftClass.IsValid())
            {
                CachedPrefabsByZone.FindOrAdd(Zone).Add(SoftClass.Get());
            }
        }
    }

    GenerateMap();
}


void AMapGenerator::AssignSpecialClusters()
{
    TSet<FIntPoint> Visited;

    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Start(X, Y);
            if (Visited.Contains(Start)) continue;
            if (!ZoneMap.Contains(Start)) continue;
            if (ZoneMap[Start].ZoneType != EZoneType::LowRise) continue;

            // Flood fill 시작
            TSet<FIntPoint> Cluster;
            TQueue<FIntPoint> Queue;
            Queue.Enqueue(Start);
            Visited.Add(Start);
            Cluster.Add(Start);

            while (!Queue.IsEmpty())
            {
                FIntPoint Current;
                Queue.Dequeue(Current);

                for (FIntPoint Offset : {FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1)})
                {
                    FIntPoint Neighbor = Current + Offset;
                    if (Visited.Contains(Neighbor)) continue;
                    if (!ZoneMap.Contains(Neighbor)) continue;

                    if (ZoneMap[Neighbor].ZoneType == EZoneType::LowRise)
                    {
                        Queue.Enqueue(Neighbor);
                        Cluster.Add(Neighbor);
                        Visited.Add(Neighbor);
                    }
                }
            }

            // 조건 만족 + 확률 충족 시 특수부지로 전체 전환
            if (Cluster.Num() >= RequiredClusterSize && RandomStream.FRand() < SpecialChance)
            {
                for (const FIntPoint& Pos : Cluster)
                {
                    ZoneMap[Pos].ZoneType = EZoneType::Special;
                }

                UE_LOG(LogTemp, Log, TEXT("Special site assigned to cluster (size = %d)"), Cluster.Num());
            }
        }
    }
}


void AMapGenerator::GenerateMap()
{
    if (CachedPrefabsByZone.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapGenerator: CachedPrefabsByZone is empty!"));
        return;
    }

    // 먼저 ZoneMap 생성
    GenerateZoneMap();

    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint GridPos(X, Y);
            if (!ZoneMap.Contains(GridPos)) continue;

            const FGridCellData& Cell = ZoneMap[GridPos];
            const TArray<TSubclassOf<AActor>>* PrefabArray = CachedPrefabsByZone.Find(Cell.ZoneType);

            // 해당 ZoneType에 대한 프리팹이 없다면 건너뜀
            if (!PrefabArray || PrefabArray->Num() == 0) continue;

            // 무작위 프리팹 선택
            int32 Index = RandomStream.RandRange(0, PrefabArray->Num() - 1);
            TSubclassOf<AActor> SelectedPrefab = (*PrefabArray)[Index];
            if (!SelectedPrefab) continue;

            // 위치 및 회전 설정
            FVector Location = GetActorLocation() + FVector(X * TileSize, Y * TileSize, 0.f);
            FRotator Rotation = Cell.PreferredRotation;

            // 액터 스폰
            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SelectedPrefab, Location, Rotation);
            if (SpawnedActor)
            {
                if (ACityBlockBase* Block = Cast<ACityBlockBase>(SpawnedActor))
                {
                    Block->SetGridPosition(GridPos);
                }

                UE_LOG(LogTemp, Log, TEXT("Spawned: %s at Grid (%d, %d), Rotation: %.0f"),
                    *SpawnedActor->GetName(), X, Y, Rotation.Yaw);
            }
        }
    }
}



void AMapGenerator::GenerateZoneMap()
{
    ZoneMap.Empty();

    // 1. 교차로 위치 설정
    int32 NumCrossroads = RandomStream.RandRange(2, 3);
    TArray<FIntPoint> Crossroads;

    for (int32 i = 0; i < NumCrossroads; ++i)
    {
        int32 X = RandomStream.RandRange(2, GridWidth - 2);
        int32 Y = RandomStream.RandRange(2, GridHeight - 2);
        Crossroads.Add(FIntPoint(X, Y));

        ZoneMap.FindOrAdd(FIntPoint(X, Y)).ZoneType = EZoneType::Road;
        ZoneMap[FIntPoint(X, Y)].bIsCrossroad = true;

        // 2. 교차로에서 사방으로 도로 확장
        for (int32 dx = 0; dx < GridWidth; ++dx)
        {
            ZoneMap.FindOrAdd(FIntPoint(dx, Y)).ZoneType = EZoneType::Road;
        }
        for (int32 dy = 0; dy < GridHeight; ++dy)
        {
            ZoneMap.FindOrAdd(FIntPoint(X, dy)).ZoneType = EZoneType::Road;
        }
    }

    // 3. 도로 인접시 고층 건물, 아니면 저층 건물
    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Pos(X, Y);

            // 이미 도로면 무시
            if (ZoneMap.Contains(Pos)) continue;

            TArray<FRotator> ValidRotations;

            for (FIntPoint Offset : {FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1)})
            {
                FIntPoint Neighbor = Pos + Offset;
                if (ZoneMap.Contains(Neighbor) && ZoneMap[Neighbor].ZoneType == EZoneType::Road)
                {
                    float Yaw = FMath::Atan2((double)Offset.Y, (double)Offset.X) * (180.0f / PI);
                    Yaw -= 90.0f;
                    ValidRotations.Add(FRotator(0.f, Yaw, 0.f));
                }
            }

            FGridCellData& Cell = ZoneMap.FindOrAdd(Pos);

            if (ValidRotations.Num() > 0)
            {
                // 도로 인접 → 고층 건물 + 무작위 방향 선택
                Cell.ZoneType = EZoneType::HighRise;
                int32 Index = RandomStream.RandRange(0, ValidRotations.Num() - 1);
                Cell.PreferredRotation = ValidRotations[Index];
            }
            else
            {
                // 도로 비접근 → 저층 건물
                Cell.ZoneType = EZoneType::LowRise;

                // 가장 가까운 도로 찾기
                float ClosestDistSq = FLT_MAX;
                FIntPoint ClosestRoad = Pos;

                for (const auto& Pair : ZoneMap)
                {
                    if (Pair.Value.ZoneType == EZoneType::Road)
                    {
                        float DistSq = FVector2D::DistSquared(FVector2D(Pair.Key), FVector2D(Pos));
                        if (DistSq < ClosestDistSq)
                        {
                            ClosestDistSq = DistSq;
                            ClosestRoad = Pair.Key;
                        }
                    }
                }

                // 방향 벡터 계산 → 입구 회전 설정
                FVector2D Dir = FVector2D(ClosestRoad - Pos).GetSafeNormal();
                float Yaw = FMath::Atan2(Dir.Y, Dir.X) * (180.0f / PI);
                Yaw -= 90.0f; // 입구가 +Y 방향일 때 보정
                Cell.PreferredRotation = FRotator(0.f, Yaw, 0.f);
            }

        }
    }

    // 4. 특수부지 설정
    AssignSpecialClusters();

    UE_LOG(LogTemp, Log, TEXT("ZoneMap generated. Total cells: %d"), ZoneMap.Num());
}

