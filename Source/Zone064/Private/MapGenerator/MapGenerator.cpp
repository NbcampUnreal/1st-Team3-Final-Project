#include "MapGenerator/MapGenerator.h"
#include "MapGenerator/CityBlockBase.h"
#include "Kismet/KismetMathLibrary.h"

AMapGenerator::AMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    // �õ�
    Seed = 42;
    // Ư������ �ּ� ũ��� Ȯ��
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

    // BlockPrefabSets -> BlockPrefabAssetsByZone ��ȯ
    for (const FZonePrefabSet& Set : BlockPrefabSets)
    {
        BlockPrefabAssetsByZone.FindOrAdd(Set.ZoneType) = Set.Prefabs;
    }

    // ����Ʈ ���۷��� �ε�
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

            // Flood fill ����
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

            // ���� ���� + Ȯ�� ���� �� Ư�������� ��ü ��ȯ
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

    // ���� ZoneMap ����
    GenerateZoneMap();

    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint GridPos(X, Y);
            if (!ZoneMap.Contains(GridPos)) continue;

            const FGridCellData& Cell = ZoneMap[GridPos];
            const TArray<TSubclassOf<AActor>>* PrefabArray = CachedPrefabsByZone.Find(Cell.ZoneType);

            // �ش� ZoneType�� ���� �������� ���ٸ� �ǳʶ�
            if (!PrefabArray || PrefabArray->Num() == 0) continue;

            // ������ ������ ����
            int32 Index = RandomStream.RandRange(0, PrefabArray->Num() - 1);
            TSubclassOf<AActor> SelectedPrefab = (*PrefabArray)[Index];
            if (!SelectedPrefab) continue;

            // ��ġ �� ȸ�� ����
            FVector Location = GetActorLocation() + FVector(X * TileSize, Y * TileSize, 0.f);
            FRotator Rotation = Cell.PreferredRotation;

            // ���� ����
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

    // 1. ������ ��ġ ����
    int32 NumCrossroads = RandomStream.RandRange(2, 3);
    TArray<FIntPoint> Crossroads;

    for (int32 i = 0; i < NumCrossroads; ++i)
    {
        int32 X = RandomStream.RandRange(2, GridWidth - 2);
        int32 Y = RandomStream.RandRange(2, GridHeight - 2);
        Crossroads.Add(FIntPoint(X, Y));

        ZoneMap.FindOrAdd(FIntPoint(X, Y)).ZoneType = EZoneType::Road;
        ZoneMap[FIntPoint(X, Y)].bIsCrossroad = true;

        // 2. �����ο��� ������� ���� Ȯ��
        for (int32 dx = 0; dx < GridWidth; ++dx)
        {
            ZoneMap.FindOrAdd(FIntPoint(dx, Y)).ZoneType = EZoneType::Road;
        }
        for (int32 dy = 0; dy < GridHeight; ++dy)
        {
            ZoneMap.FindOrAdd(FIntPoint(X, dy)).ZoneType = EZoneType::Road;
        }
    }

    // 3. ���� ������ ���� �ǹ�, �ƴϸ� ���� �ǹ�
    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Pos(X, Y);

            // �̹� ���θ� ����
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
                // ���� ���� �� ���� �ǹ� + ������ ���� ����
                Cell.ZoneType = EZoneType::HighRise;
                int32 Index = RandomStream.RandRange(0, ValidRotations.Num() - 1);
                Cell.PreferredRotation = ValidRotations[Index];
            }
            else
            {
                // ���� ������ �� ���� �ǹ�
                Cell.ZoneType = EZoneType::LowRise;

                // ���� ����� ���� ã��
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

                // ���� ���� ��� �� �Ա� ȸ�� ����
                FVector2D Dir = FVector2D(ClosestRoad - Pos).GetSafeNormal();
                float Yaw = FMath::Atan2(Dir.Y, Dir.X) * (180.0f / PI);
                Yaw -= 90.0f; // �Ա��� +Y ������ �� ����
                Cell.PreferredRotation = FRotator(0.f, Yaw, 0.f);
            }

        }
    }

    // 4. Ư������ ����
    AssignSpecialClusters();

    UE_LOG(LogTemp, Log, TEXT("ZoneMap generated. Total cells: %d"), ZoneMap.Num());
}

