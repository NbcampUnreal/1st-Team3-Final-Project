#include "MapGenerator/MapGenerator.h"
#include "MapGenerator/CityBlockBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

AMapGenerator::AMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    GridWidth = 10;
    GridHeight = 10;
    TileSize = 500;

    // 시드
    Seed = 42;
    // 특수부지 최소 크기와 확률
    RequiredClusterSize = 6;
    SpecialChance = 0.2f;
    CrossroadMinSpacing = 4;
    CrosswalkChance = 0.3f;

    // 도로 프랍 스폰 확률
    TreeSpawnChance = 0.3f;
    LightSpawnChance = 0.3f;
    TrashSpawnChance = 0.2f;
    TrafficSpawnChance = 0.5f;


    LightSpawnSpacing = 4;
    TreeSpawnSpacing = 5;

    SearchOffsetList = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
    CornerOffsetList = { FIntPoint(1, 1), FIntPoint(-1, 1), FIntPoint(1, 1), FIntPoint(-1, -1) };
    CornerYawMap = { {FIntPoint(1, 1), -180.f}, { FIntPoint(-1, -1), 0.f }, { FIntPoint(-1, 1), -90.f }, { FIntPoint(1, -1), 90.f } };
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

                for (FIntPoint Offset : SearchOffsetList)
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

bool AMapGenerator::IsAreaAvailable(FIntPoint TopLeft, int32 Width, int32 Height, const TArray<EZoneType>& BlockedTypes)
{
    // 그리드 바깥으로 나가지 않는지 검사
    if (TopLeft.X < 0
        || TopLeft.Y < 0
        || TopLeft.X + Width > GridWidth
        || TopLeft.Y + Height > GridHeight)
    {
        return false;
    }

    // 이미 사용된 셀이 있는지 검사
    for (int32 dx = 0; dx < Width; ++dx)
    {
        for (int32 dy = 0; dy < Height; ++dy)
        {
            FIntPoint Pos = TopLeft + FIntPoint(dx, dy);
            if (ZoneMap.Contains(Pos))
            {
                return false;
            }
        }
    }
    return true;
}


void AMapGenerator::MarkZone(FIntPoint TopLeft, int32 Width, int32 Height, EZoneType ZoneType, FRotator Rotation)
{
    for (int32 dx = 0; dx < Width; ++dx)
    {
        for (int32 dy = 0; dy < Height; ++dy)
        {
            FIntPoint Pos = TopLeft + FIntPoint(dx, dy);
            FGridCellData& Cell = ZoneMap.FindOrAdd(Pos);
            Cell.ZoneType = ZoneType;
            Cell.PreferredRotation = Rotation;
        }
    }
}

void AMapGenerator::DrawDebugZoneMap()
{
    if (!GetWorld()) return;

    for (const auto& Pair : ZoneMap)
    {
        FIntPoint GridPos = Pair.Key;
        const FGridCellData& Cell = Pair.Value;

        FVector Center = GetActorLocation() + FVector(GridPos.X * TileSize, GridPos.Y * TileSize, 50.f); // Z 높이 보정
        FVector BoxExtent = FVector(TileSize * 0.5f, TileSize * 0.5f, 50.f); // 두께는 100으로 고정

        FColor Color;

        switch (Cell.ZoneType)
        {
        case EZoneType::Road:            Color = FColor::Silver; break;
        case EZoneType::HighRise:        Color = FColor::Blue;   break;
        case EZoneType::LowRise:         Color = FColor::Green;  break;
        case EZoneType::Special:         Color = FColor::Red;    break;
        case EZoneType::Road_Sidewalk:   Color = FColor::Orange; break;
        case EZoneType::Road_Crosswalk:  Color = FColor::Yellow; break;
        case EZoneType::Road_Intersection: Color = FColor::Cyan; break;
        default:                         Color = FColor::Black;  break;
        }

        DrawDebugBox(GetWorld(), Center, BoxExtent, Cell.PreferredRotation.Quaternion(), Color, true, -1.f, 0, 4.f);

        if (Cell.bIsCrossroad)
        {
            DrawDebugString(GetWorld(), Center + FVector(0, 0, 100), TEXT("X"), nullptr, FColor::White, 0.f, true);
        }
    }
}

void AMapGenerator::SpawnSidewalkProps()
{
    for (const auto& Pair : ZoneMap)
    {
        if (Pair.Value.ZoneType != EZoneType::Road_Sidewalk)
        {
            if (Pair.Value.ZoneType != EZoneType::Alley)
            {
                if (Pair.Value.ZoneType != EZoneType::Road_Sidewalk_Traffic)
                {
                    continue;
                }
            }
        }

        FIntPoint GridPos = Pair.Key;
        FVector WorldLocation = GetActorLocation() + FVector(GridPos.X * TileSize, GridPos.Y * TileSize, 0.f);

        // AABB 검사로 해당 위치 프리팹 찾기
        FBox QueryBox = FBox::BuildAABB(WorldLocation, FVector(100.f, 100.f, 100.f));
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

        for (AActor* Actor : FoundActors)
        {
            if (Actor && QueryBox.IsInside(Actor->GetActorLocation()))
            {
                TrySpawnProps(Actor, GridPos); 
                break;
            }
        }
    }
}


void AMapGenerator::TrySpawnProps(AActor* Target, FIntPoint GridPos)
{
    if (!Target || !Target->IsValidLowLevel()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<USceneComponent*> Components;
    Target->GetComponents<USceneComponent>(Components);

    for (USceneComponent* Comp : Components)
    {
        if (!Comp) continue;

        FString Name = Comp->GetName().ToLower();

        TArray<TSubclassOf<AActor>>* PropArray = nullptr;
        float Chance = 1.f;

        if (Name.Contains(TEXT("Tree")))
        {
            if ((GridPos.X + GridPos.Y) % TreeSpawnSpacing != 0) continue;

            PropArray = &TreePrefabs;
            Chance = TreeSpawnChance;
        }
        else if (Name.Contains(TEXT("Light")))
        {
            if ((GridPos.X + GridPos.Y) % LightSpawnSpacing != 0) continue;
            
            PropArray = &LightPrefabs;
            Chance = LightSpawnChance;
        }
        else if (Name.Contains(TEXT("Trash")))
        {
            PropArray = &TrashPrefabs;
            Chance = TrashSpawnChance;
        }
        else if (Name.Contains(TEXT("Traffic")))
        {
            PropArray = &TrafficPrefabs;
            Chance = TrafficSpawnChance;
        }

        if (PropArray && PropArray->Num() > 0 && RandomStream.FRand() < Chance)
        {
            int32 Index = RandomStream.RandRange(0, PropArray->Num() - 1);
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

FIntPoint AMapGenerator::GetTopLeftFromOrigin(FIntPoint Origin, int32 Width, int32 Height, const FRotator& Rotation)
{
    int32 DX = 0;
    int32 DY = 0;

    float Yaw = FMath::Fmod(Rotation.Yaw, 360.f);
    if (Yaw < 0.f)
        Yaw += 360.f;

    if (FMath::IsNearlyEqual(Yaw, 0.f))
    {
        DX = 0;
        DY = 0;
    }
    else if (FMath::IsNearlyEqual(Yaw, 90.f))
    {
        DX = 0;
        DY = -(Width - 1);
    }
    else if (FMath::IsNearlyEqual(Yaw, 180.f))
    {
        DX = -(Width - 1);
        DY = -(Height - 1);
    }
    else if (FMath::IsNearlyEqual(Yaw, 270.f))
    {
        DX = -(Height - 1);
        DY = 0;
    }

    return Origin + FIntPoint(DX, DY);
}




FVector AMapGenerator::GetWorldCenterFromTopLeft(FIntPoint TopLeft, int32 Width, int32 Height, FRotator Rotation)
{
    FVector LocalOffset(Width * 0.5f * TileSize, Height * 0.5f * TileSize, 0.f);

    // 좌상단 모서리 -> 월드 위치
    FVector WorldTopLeft = GetActorLocation() + FVector(TopLeft.X * TileSize, TopLeft.Y * TileSize, 0.f);

    // 회전 적용
    return WorldTopLeft + Rotation.RotateVector(LocalOffset);
}

FVector AMapGenerator::GetWorldFromGrid(FIntPoint GridPos)
{
    return GetActorLocation() + FVector(GridPos.X * TileSize, GridPos.Y * TileSize, 0.f);
}




void AMapGenerator::GenerateMap()
{
    if (CachedPrefabsByZone.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapGenerator: CachedPrefabsByZone is empty!"));
        return;
    }

    GenerateZoneMap(); // ZoneMap + BuildingSpawnList 구성

    // 도로 및 인도 (1x1 크기)스폰 
    for (const auto& Pair : ZoneMap)
    {
        const FIntPoint& GridPos = Pair.Key;
        const FGridCellData& Cell = Pair.Value;

        // 건물은 건너뜀 (HighRise, LowRise, Special)
        if (Cell.ZoneType == EZoneType::HighRise ||
            Cell.ZoneType == EZoneType::LowRise ||
            Cell.ZoneType == EZoneType::Special)
        {
            continue;
        }

        const TArray<TSubclassOf<AActor>>* PrefabArray = CachedPrefabsByZone.Find(Cell.ZoneType);
        if (!PrefabArray || PrefabArray->Num() == 0) continue;

        int32 Index = RandomStream.RandRange(0, PrefabArray->Num() - 1);
        TSubclassOf<AActor> Selected = (*PrefabArray)[Index];
        if (!Selected) continue;

        FVector Location = GetActorLocation() + FVector(GridPos.X * TileSize, GridPos.Y * TileSize, 0.f);
        FRotator Rotation = Cell.PreferredRotation;

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Selected, Location, Rotation);
        if (ACityBlockBase* Block = Cast<ACityBlockBase>(SpawnedActor))
        {
            Block->InitializeBlock(GridPos, Cell.bIsCrossroad, Cell.RoadDirection);
        }

        UE_LOG(LogTemp, Log, TEXT("Spawned (infra): %s at (%d, %d)"), *Selected->GetName(), GridPos.X, GridPos.Y);
    }

    // 큐브메시 박스로 실험
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh) return;

    for (const FBuildingSpawnData& Info : BuildingSpawnList)
    {
        // 여백 설정 (건물 사이로 골목이 보이게)
        float PaddingX = FMath::FRandRange(0.00f, 0.06f);
        float PaddingY = FMath::FRandRange(0.00f, 0.03f);

        // 높이 결정
        float HeightZ = (Info.ZoneType == EZoneType::HighRise) ? 1800.f + (200.f * FMath::RandRange(1, 5)) :
            (Info.ZoneType == EZoneType::LowRise) ? 500.f + (200.f * FMath::RandRange(1, 3)) : 300.f;

        // 구역 크기와 실제 건물 크기
        float BlockX = Info.Width * TileSize;
        float BlockY = Info.Height * TileSize;
        float InnerX = BlockX * (1.0f - PaddingX * 2.0f);
        float InnerY = BlockY * (1.0f - PaddingY * 2.0f);

        // 크기(스케일) 계산
        FVector Scale(
            InnerX / 100.f,
            InnerY / 100.f,
            HeightZ / 100.f
        );

        // Top-Left 셀 중심 얻기
        FVector TopLeftCenter = GetWorldFromGrid(Info.Origin);

        // 블록 중심 오프셋: (Width-1)/2, (Height-1)/2 만큼
        FVector CenterOffset(
            (Info.Width - 1) * 0.5f * TileSize,
            (Info.Height - 1) * 0.5f * TileSize,
            HeightZ * 0.5f
        );

        // 최종 스폰 위치 (회전 영향 x)
        FVector SpawnLocation = TopLeftCenter + CenterOffset;

        // 액터 스폰 (Rotation 은 여기서만 적용)
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            SpawnLocation,
            Info.Rotation
        );
        if (MeshActor)
        {
            auto* MeshComp = MeshActor->GetStaticMeshComponent();
            MeshComp->SetMobility(EComponentMobility::Movable);
            MeshComp->SetStaticMesh(CubeMesh);
            MeshComp->SetWorldScale3D(Scale);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }


    //// 건물 스폰 (영역 단위)
    //for (const FBuildingSpawnData& Info : BuildingSpawnList)
    //{
    //    const TArray<TSubclassOf<AActor>>* PrefabArray = CachedPrefabsByZone.Find(Info.ZoneType);
    //    if (!PrefabArray || PrefabArray->Num() == 0) continue;

    //    // 크기 일치하는 프리팹만 필터링 (추가 가능)
    //    int32 Index = RandomStream.RandRange(0, PrefabArray->Num() - 1);
    //    TSubclassOf<AActor> Selected = (*PrefabArray)[Index];
    //    if (!Selected) continue;

    //    FVector Location = GetActorLocation() +
    //        FVector(Info.Origin.X * TileSize + (Info.Width - 1) * 0.5f * TileSize,
    //            Info.Origin.Y * TileSize + (Info.Height - 1) * 0.5f * TileSize,
    //            0.f);

    //    AActor* Spawned = GetWorld()->SpawnActor<AActor>(Selected, Location, Info.Rotation);

    //    if (ACityBlockBase* Block = Cast<ACityBlockBase>(Spawned))
    //    {
    //        Block->InitializeBlock(Info.Origin, false, ERoadDirection::None);
    //    }

    //    UE_LOG(LogTemp, Log, TEXT("Spawned (building): %s at (%d,%d) size (%d x %d)"),
    //        *Selected->GetName(), Info.Origin.X, Info.Origin.Y, Info.Width, Info.Height);
    //}

    // 도로 프랍 스폰
    SpawnSidewalkProps();
}




void AMapGenerator::GenerateZoneMap()
{
    ZoneMap.Empty();

    // 1. 교차로 생성
    int32 NumCrossroads = RandomStream.RandRange(2, 2);  // 교차로 개수 설정-> 현재 2개 고정
    TArray<FIntPoint> CrossroadCenters;
    TSet<FIntPoint> UsedRoadCells;  // 교차로 확장되면서 겹치는 곳 교차로로 만들기 위해
    CrossroadMinSpacing = 6; // 교차로 간 최소 간격
    int32 RetryCount = 0;
    int32 MaxRetry = 100; // 최대 재시도 횟수

    TArray<EZoneType> Blocked = {
        EZoneType::HighRise,
        EZoneType::LowRise,
        EZoneType::Special,
        EZoneType::Road,
        EZoneType::Road_Sidewalk
    };

    while (CrossroadCenters.Num() < NumCrossroads && RetryCount < MaxRetry)
    {
        ++RetryCount;

        int32 Margin = 3;
        int32 MinX = Margin, MinY = Margin;
        int32 MaxX = GridWidth - Margin - 1;
        int32 MaxY = GridHeight - Margin - 1;

        int32 CenterX = RandomStream.RandRange(MinX, MaxX);
        int32 CenterY = RandomStream.RandRange(MinY, MaxY);
        FIntPoint Center(CenterX, CenterY);

        // 기존 교차로들과 거리 검사
        bool bTooClose = false;
        for (const FIntPoint& Existing : CrossroadCenters)
        {
            if (FMath::Abs(CenterX - Existing.X) < CrossroadMinSpacing || FMath::Abs(CenterY - Existing.Y) < CrossroadMinSpacing)
            {
                bTooClose = true;
                break;
            }
        }
        if (bTooClose) continue;

        // 교차로 크기 확률에 따라 결정
        float Rand = RandomStream.FRand(); 
        int32 CrossroadSize = 1;  
        if (Rand < 0.2f)
            CrossroadSize = 1;  // 20% 1x1
        else if (Rand < 0.6f)
            CrossroadSize = 2;  // 40% 2x2
        else
            CrossroadSize = 3;  // 40% 3x3

        int32 Half = CrossroadSize / 2;
        FIntPoint TopLeft = Center - FIntPoint(Half, Half);
        UE_LOG(LogTemp, Warning, TEXT("Crossroad Size : %i"), CrossroadSize);
        if (!IsAreaAvailable(TopLeft, CrossroadSize, CrossroadSize, Blocked))
            continue;

        // 교차로 마킹
        for (int32 dx = 0; dx < CrossroadSize; ++dx)
        {
            for (int32 dy = 0; dy < CrossroadSize; ++dy)
            {
                FIntPoint Pos = TopLeft + FIntPoint(dx, dy);
                FGridCellData& Cell = ZoneMap.FindOrAdd(Pos);
                Cell.ZoneType = EZoneType::Road;
                Cell.bIsCrossroad = true;
                Cell.CrossroadSize = CrossroadSize;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Crossroad Size: %d TopLeft: (%d,%d) Center: (%d,%d)"),
            CrossroadSize, TopLeft.X, TopLeft.Y, Center.X, Center.Y);

        for (int32 dx = 0; dx < CrossroadSize; ++dx) {
            for (int32 dy = 0; dy < CrossroadSize; ++dy) {
                FIntPoint P = TopLeft + FIntPoint(dx, dy);
                UE_LOG(LogTemp, Warning, TEXT("Crossroad Covers: (%d,%d)"), P.X, P.Y);
            }
        }

        //  교차로 둘레 횡단보도 설정
        if (CrossroadSize > 1)
        {
            for (int32 dx = -1; dx <= CrossroadSize; dx++)
            {
                for (int32 dy = -1; dy <= CrossroadSize; dy++)
                {
                    bool IsBorder =
                        ((dx == -1 || dx == CrossroadSize) && (dy >= 0 && dy < CrossroadSize)) ||
                        ((dy == -1 || dy == CrossroadSize) && (dx >= 0 && dx < CrossroadSize));
                    if (!IsBorder) continue;
                    FIntPoint Outer = TopLeft + FIntPoint(dx, dy);
                    if (!ZoneMap.Contains(Outer))
                    {
                        //  이 시점에 ZoneMap에는 교차로 말고는 없으므로 없으면 새로 추가하면서 지정
                        FGridCellData& Cell = ZoneMap.FindOrAdd(Outer);
                        Cell.ZoneType = EZoneType::Road_Crosswalk;
                        float CrosswalkRotatorYaw = (dx == -1 || dx == CrossroadSize) ? 0.f : 90.f;
                        Cell.PreferredRotation = FRotator(0.f, CrosswalkRotatorYaw, 0.f);
                    }
                }
            }
        }

        CrossroadCenters.Add(Center);

        // 교차로를 기준으로 도로 확장
        for(int32 dx = -Half; dx < Half + (CrossroadSize % 2 == 1 ? 1 : 0); ++dx)
        {
            for (int32 x = 0; x < GridWidth; ++x)
            {
                FIntPoint RoadPos(x, Center.Y + dx);
                if (ZoneMap.Contains(RoadPos)) 
                {
                    // 이미 다른 도로가 있으면 교차로로 설정
                    FGridCellData& Cell = ZoneMap[RoadPos];
                    if (Cell.ZoneType == EZoneType::Road_Crosswalk) continue;
                    Cell.bIsCrossroad = true;
                    Cell.ZoneType = EZoneType::Road;
                    continue;
                }
                FGridCellData& Cell = ZoneMap.FindOrAdd(RoadPos);
                Cell.ZoneType = EZoneType::Road;
                Cell.CrossroadSize = CrossroadSize;
            }

            for (int32 y = 0; y < GridHeight; ++y)
            {
                FIntPoint RoadPos(Center.X + dx, y);
                if (ZoneMap.Contains(RoadPos))
                {
                    // 이미 다른 도로가 있으면 교차로로 설정
                    FGridCellData& Cell = ZoneMap[RoadPos];
                    if (Cell.ZoneType == EZoneType::Road_Crosswalk) continue;
                    Cell.bIsCrossroad = true;
                    Cell.ZoneType = EZoneType::Road;
                    continue;
                }
                FGridCellData& Cell = ZoneMap.FindOrAdd(RoadPos);
                Cell.ZoneType = EZoneType::Road;
                Cell.CrossroadSize = CrossroadSize;
            }
        }
    }

    // 2. 도로 주변 인도 추가
    TArray<FIntPoint> RoadCells;
    for (const auto& Pair : ZoneMap)
    {
        if (Pair.Value.ZoneType == EZoneType::Road || Pair.Value.ZoneType == EZoneType::Road_Crosswalk)
        {
            RoadCells.Add(Pair.Key);
        }
    }

    for (const FIntPoint& Pos : RoadCells)
    {
        for (const FIntPoint& Offset : SearchOffsetList)
        {
            FIntPoint SidePos = Pos + Offset;

            // 경계 검사
            if (SidePos.X < 0 || SidePos.X >= GridWidth ||
                SidePos.Y < 0 || SidePos.Y >= GridHeight)
            {
                continue;
            }
            //  인도 지정
            if (!ZoneMap.Contains(SidePos))
            {
                // 교차로 꼭짓점 체크
                bool bIsTrafficCorner = false;
                float TrafficYaw = 0.f;
                for (const FIntPoint& CornerOffset : CornerOffsetList)
                {
                    FIntPoint MaybeCrossroad = SidePos + CornerOffset;
                    if (ZoneMap.Contains(MaybeCrossroad) && ZoneMap[MaybeCrossroad].bIsCrossroad && ZoneMap[MaybeCrossroad].CrossroadSize > 1)
                    {
                        bIsTrafficCorner = true;
                        if (CornerYawMap.Contains(CornerOffset))
                        {
                            TrafficYaw = CornerYawMap[CornerOffset];
                        }
                        break;
                    }
                }

                FGridCellData& Cell = ZoneMap.Add(SidePos);

                if (bIsTrafficCorner)
                {
                    Cell.ZoneType = EZoneType::Road_Sidewalk_Traffic;
                    Cell.PreferredRotation = FRotator(0.f, TrafficYaw, 0.f);
                }
                else
                {
                    Cell.ZoneType = EZoneType::Road_Sidewalk;
                    float Yaw = FMath::Atan2((float)Offset.Y, (float)Offset.X) * 180.f / PI - 90.f;
                    Cell.PreferredRotation = FRotator(0.f, Yaw, 0.f);
                }
            }
        }
    }

    // 3. 도로 방향 설정
    for (auto& Pair : ZoneMap)
    {
        if (Pair.Value.ZoneType != EZoneType::Road && Pair.Value.ZoneType != EZoneType::Road_Crosswalk) continue;

        if (Pair.Value.bIsCrossroad)
        {
            Pair.Value.RoadDirection = ERoadDirection::Crossroad;
            continue;
        }

        FIntPoint Pos = Pair.Key;

        // b{방향} == true -> {방향}쪽 그리드가 도로이다
        bool bLeft = ZoneMap.Contains(Pos + FIntPoint(-1, 0)) && (ZoneMap[Pos + FIntPoint(-1, 0)].ZoneType == EZoneType::Road || ZoneMap[Pos + FIntPoint(-1, 0)].ZoneType == EZoneType::Road_Crosswalk);
        bool bRight = ZoneMap.Contains(Pos + FIntPoint(1, 0)) && (ZoneMap[Pos + FIntPoint(1, 0)].ZoneType == EZoneType::Road || ZoneMap[Pos + FIntPoint(1, 0)].ZoneType == EZoneType::Road_Crosswalk);
        bool bUp = ZoneMap.Contains(Pos + FIntPoint(0, 1)) && (ZoneMap[Pos + FIntPoint(0, 1)].ZoneType == EZoneType::Road || ZoneMap[Pos + FIntPoint(0, 1)].ZoneType == EZoneType::Road_Crosswalk);
        bool bDown = ZoneMap.Contains(Pos + FIntPoint(0, -1)) &&( ZoneMap[Pos + FIntPoint(0, -1)].ZoneType == EZoneType::Road || ZoneMap[Pos + FIntPoint(0, -1)].ZoneType == EZoneType::Road_Crosswalk);

        if ((bLeft || bRight) && !(bUp || bDown))
        {
            Pair.Value.RoadDirection = ERoadDirection::Horizontal;
        }
        else if ((bUp || bDown) && !(bLeft || bRight))
        {
            Pair.Value.RoadDirection = ERoadDirection::Vertical;
        }
        /*else 
        {
            Pair.Value.RoadDirection = ERoadDirection::Crossroad;
        }*/
    }

    /*  후 생성 횡단보도 만들기
    *   1. 전체 ZoneMap 순회 - >EZoneType == Road인 셀만 순회 
    *   2. 4방향 셀 조회 -> ERoadDirection == Crossroad 라면 현재 그리드 ZoneType을 Road_Crosswalk로 변경
    *   3. 현재 그리드와 교차로의 상대 위치에 따라 PrefferdRotation 설정
    *   3-1. CrossroadSize  > 1 무조건 횡단보도 설치
    *   3-2. 1칸 짜리 도로면 확률에 따라 횡단보도 설치
    */
    for (auto& Pair : ZoneMap)
    {
        //  EZoneType::Road 만 순회
        if (Pair.Value.ZoneType != EZoneType::Road) continue;
        if (Pair.Value.bIsCrossroad == true) continue;
        if (Pair.Value.ZoneType == EZoneType::Road_Crosswalk) continue;

        FIntPoint CurrentGrid = Pair.Key;

        for (auto& Offset : SearchOffsetList)
        {
            FIntPoint SearchOffset = CurrentGrid + Offset;
            if (ZoneMap.Contains(SearchOffset) && ZoneMap[SearchOffset].bIsCrossroad == true && ZoneMap[CurrentGrid].ZoneType != EZoneType::Road_Crosswalk)
            {
                if (ZoneMap[CurrentGrid].CrossroadSize >= 2 || RandomStream.FRand() < CrosswalkChance)
                {
                    FGridCellData& Cell = ZoneMap[CurrentGrid];
                    Cell.ZoneType = EZoneType::Road_Crosswalk;
                    /*  아래 방법으로 회전시키려고 했는데 아무리 해봐도 RoadDirection이 none으로 나옴
                    *   CurrentGrid 기준으로 bIsCrossroad == true인 SearchOffset 방향으로 회전시켜야 겠음
                    */
                    //float RotationYaw = (Cell.RoadDirection == ERoadDirection::Horizontal) ? 0.f : 90.f;
                    float RotationYaw = (Offset.X != 0 && Offset.Y == 0) ? 0.f : 90.f;
                    Cell.PreferredRotation = FRotator(0, RotationYaw, 0);

                }

            }
        }
    }


    // 4. 고층 건물 배치
    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Pos(X, Y);
            // 이미 어떤 구역이라도 마킹되어 있으면 건너뛰기
            if (ZoneMap.Contains(Pos))
                continue;

            // 1) 인도 주변 셀인지 검사하고 유효한 회전 리스트 수집
            bool bNearSidewalk = false;
            TArray<FRotator> ValidRotations;
            for (const FIntPoint& Offset : SearchOffsetList)
            {
                FIntPoint N = Pos + Offset;
                if (ZoneMap.Contains(N) && ZoneMap[N].ZoneType == EZoneType::Road_Sidewalk)
                {
                    bNearSidewalk = true;
                    float Yaw = FMath::Atan2((float)Offset.Y, (float)Offset.X) * 180.f / PI - 90.f;
                    ValidRotations.Add(FRotator(0.f, Yaw, 0.f));
                }
            }
            if (!bNearSidewalk || ValidRotations.Num() == 0)
                continue;

            // 2) 고층 크기 랜덤 (가로 S1, 세로 S2)
            int32 S1 = RandomStream.RandRange(3, 5);
            //int32 S2 = RandomStream.RandRange(3, 3);

            // 3) 회전 랜덤 선택
            int32 RotIdx = RandomStream.RandRange(0, ValidRotations.Num() - 1);
            FRotator Rotation = ValidRotations[RotIdx];

            // 4) 회전에 따른 가로/세로 결정
            bool bRotated = FMath::Abs(Rotation.Yaw) == 90.f;
            int32 Width = bRotated ? S1 : S1;
            int32 Height = bRotated ? S1 : S1;

            // 5) Top-Left 그리드 좌표 계산
            FIntPoint TopLeft = GetTopLeftFromOrigin(Pos, Width, Height, Rotation);

            // 6) 영역 사용 가능 여부 검사
            if (!IsAreaAvailable(TopLeft, Width, Height, /*BlockedTypes=*/{}))
                continue;

            // 7) 마킹 및 스폰 리스트에 추가
            MarkZone(TopLeft, Width, Height, EZoneType::HighRise, Rotation);
            AddtoBuildingSpawnList(TopLeft, Width, Height, EZoneType::HighRise, Rotation);
        }
    }

    // 5. 남은 빈 공간에 저층 배치
    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Pos(X, Y);
            if (ZoneMap.Contains(Pos)) continue;  // 이미 마킹된 셀 건너뛰기

            // 회전 계산
            int32 Size = 2;
            float ClosestDistSq = FLT_MAX;
            FIntPoint ClosestSidewalk = Pos;

            // 1) 가장 가까운 인도(Road_Sidewalk) 셀 찾기
            for (const auto& Pair : ZoneMap)
            {
                if (Pair.Value.ZoneType == EZoneType::Road_Sidewalk)
                {
                    float DistSq = FVector2D::DistSquared(
                        FVector2D(Pair.Key), FVector2D(Pos));
                    if (DistSq < ClosestDistSq)
                    {
                        ClosestDistSq = DistSq;
                        ClosestSidewalk = Pair.Key;
                    }
                }
            }

            // 2) 방향 벡터 → Yaw 계산
            FVector2D Dir = (FVector2D(ClosestSidewalk) - FVector2D(Pos)).GetSafeNormal();
            float Yaw = FMath::Atan2(Dir.Y, Dir.X) * 180.f / PI - 90.f;
            FRotator Rot(0.f, Yaw, 0.f);

            // 3) TopLeft 구하고 배치 가능 여부 검사
            FIntPoint TL = GetTopLeftFromOrigin(Pos, Size, Size, Rot);
            if (IsAreaAvailable(TL, Size, Size, /*BlockedTypes*/{}))
            {
                MarkZone(TL, Size, Size, EZoneType::LowRise, Rot);
                AddtoBuildingSpawnList(TL, Size, Size, EZoneType::LowRise, Rot);
            }
        }
    }

    // 5. 특수 부지 지정
    AssignSpecialClusters();

    // 6. 남은 빈 칸에 화단 or 뒷골목 스폰
    for (int32 X = 0; X < GridWidth; ++X)
    {
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            FIntPoint Pos(X, Y);
            // 이미 ZoneMap에 마킹된 셀은 스킵
            if (ZoneMap.Contains(Pos))
                continue;

            // 인도 인접 여부 판단
            bool bNearSidewalk = false;
            for (const FIntPoint& Offset : SearchOffsetList)
            {
                if (ZoneMap.Contains(Pos + Offset) && ZoneMap[Pos + Offset].ZoneType == EZoneType::Road_Sidewalk)
                {
                    bNearSidewalk = true;
                    break;
                }
            }

            // 인도 인접시 화단, 아니면 뒷골목
            if (bNearSidewalk)
            {
                FGridCellData& Cell = ZoneMap.Add(Pos);
                Cell.ZoneType = EZoneType::Plant;
                Cell.PreferredRotation = FRotator::ZeroRotator;
            }
            else
            {
                FGridCellData& Cell = ZoneMap.Add(Pos);
                Cell.ZoneType = EZoneType::Alley;
                Cell.PreferredRotation = FRotator::ZeroRotator;
            }
            
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ZoneMap generated. Total cells: %d"), ZoneMap.Num());
}

void AMapGenerator::AddtoBuildingSpawnList(FIntPoint Pos, int32 Width, int32 Height, EZoneType ZoneType, FRotator Rotation)
{
    FBuildingSpawnData Info;
    Info.Origin = Pos;
    Info.Width = Width;
    Info.Height = Height;
    Info.ZoneType = ZoneType;
    Info.Rotation = Rotation;

    BuildingSpawnList.Add(Info);
}


