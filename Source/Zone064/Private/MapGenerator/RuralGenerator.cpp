// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/RuralGenerator.h"
#include "Kismet/GameplayStatics.h"

ARuralGenerator::ARuralGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

    GridWidth = 30;
    GridHeight = 30;
    TileSize = 500.f;

    Seed = 42;
    MaxShopCount = 7;
    MaxHouseCount = 20;
    MaxFiveByFiveShopCount = 1;

    AlleyLightSpawnChance = 0.4f;

    bHouseLimitReached = false;
    bShopLimitReached = false;

    FurtherLeftRightOffsetList = { FIntPoint(1, 0), FIntPoint(2, 0), FIntPoint(-1, 0), FIntPoint(-2, 0) };
    UpDownOffsetList = { FIntPoint(0, 1), FIntPoint(0, -1) };

    SearchOffsetList = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
    CornerOffsetList = { FIntPoint(1, 1), FIntPoint(-1, 1), FIntPoint(1, -1), FIntPoint(-1, -1) };
    CornerYawMap = { {FIntPoint(1, 1), -180.f}, { FIntPoint(-1, -1), 0.f }, { FIntPoint(-1, 1), -90.f }, { FIntPoint(1, -1), 90.f } };

}

void ARuralGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}



void ARuralGenerator::GenerateZoneMap()
{
    ZoneMap.Empty();

    // 1. 교차로 생성
    int32 NumCrossroads = 1;  // 교차로 개수 설정 : 1개
    TArray<FIntPoint> CrossroadCenters;
    TSet<FIntPoint> UsedRoadCells;  // 교차로 확장되면서 겹치는 곳 교차로로 만들기 위해 : 필요 없음
    CrossroadMinSpacing = 6; // 교차로 간 최소 간격
    int32 RetryCount = 0;
    int32 MaxRetry = 100; // 최대 재시도 횟수
    FIntPoint TopLeft = FIntPoint(0, 0); // 교차로 시작 지점
    int32 CrossroadSize = 1; // 교차로 크기

    TArray<EZoneType> Blocked = {
        EZoneType::HighRise3,
        EZoneType::HighRise4,
        EZoneType::HighRise5,
        EZoneType::LowRise,
        EZoneType::Special,
        EZoneType::Road,
        EZoneType::Road_Sidewalk
    };

    while (CrossroadCenters.Num() < NumCrossroads && RetryCount < MaxRetry)
    {
        ++RetryCount;

        // 교차로 1개 생성인데 너무 구석에 생성되지 않도록 중간값 내외에서 결정
        int32 Margin = 4;
        int32 Middle = (GridWidth >= GridHeight) ? GridWidth / 2 : GridHeight / 2;
        int32 MinX = Middle - Margin;
        int32 MaxX = Middle + Margin;
        int32 MinY = Middle - Margin;
        int32 MaxY = Middle + Margin;

        int32 CenterX = FMath::RandRange(MinX, MaxX);
        int32 CenterY = FMath::RandRange(MinY, MaxY);
        FIntPoint Center(CenterX, CenterY);

        // 교차로 크기 확률에 따라 결정
        float Rand = FMath::FRand();
        if (Rand < 0.5f)
            CrossroadSize = 1;  // 50% 1x1
        else
            CrossroadSize = 2;  // 50% 2x2
        
        int32 Half = CrossroadSize / 2;
        TopLeft = Center - FIntPoint(Half, Half);
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
        for (int32 dx = -Half; dx < Half + (CrossroadSize % 2 == 1 ? 1 : 0); ++dx)
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

    //---- 플레이어 스타트 배치 ----//
    int32 PlayerStartIndex = RandomStream.RandRange(0, CrossroadCenters.Num() - 1);
    FIntPoint PlayerStartPos = CrossroadCenters[PlayerStartIndex];

    FVector PlayerStartLocation = GetActorLocation() + FVector(PlayerStartPos.X * TileSize, PlayerStartPos.Y * TileSize, 50.f);
    FRotator PlayerStartRotation = FRotator(0.f, RandomStream.FRandRange(-180.f, 180.f), 0.f);

    GetWorld()->SpawnActor<AActor>(PlayerStartActor, PlayerStartLocation, PlayerStartRotation);


    /*
    *   교차로 기준 사분면에 각각 영역 지정 (TopLeft 활용)
    *   각 사분면의 시작과 끝 좌표 기록
    *   가장 작은 부분, 2번째로 큰 부분 경작지로 설정
    */

    FIntPoint FirstStart = FIntPoint(0, 0);
    FIntPoint FirstEnd = FIntPoint(TopLeft.X - 1, TopLeft.Y - 1);
    FIntPoint SecondStart = FIntPoint(TopLeft.X + CrossroadSize, 0);
    FIntPoint SecondEnd = FIntPoint(GridWidth - 1, TopLeft.Y - 1);
    FIntPoint ThirdStart = FIntPoint(0, TopLeft.Y + CrossroadSize);
    FIntPoint ThirdEnd = FIntPoint(TopLeft.X - 1, GridHeight - 1);
    FIntPoint FourthStart = FIntPoint(TopLeft.X + CrossroadSize, TopLeft.Y + CrossroadSize);
    FIntPoint FourthEnd = FIntPoint(GridWidth - 1, GridHeight - 1);

    TArray<TPair<FIntPoint, FIntPoint>> Offsets;
    Offsets = { {FirstStart, FirstEnd}, {SecondStart, SecondEnd}, {ThirdStart, ThirdEnd}, {FourthStart, FourthEnd} };

    TArray<int32> Areas;
    for (const TPair<FIntPoint, FIntPoint>& Range : Offsets)
    {
        int32 Area = (Range.Value.X - Range.Key.X + 1) * (Range.Value.Y - Range.Key.Y + 1);
        Areas.Add(Area);
    }

    TArray<int32> SortedIndices = { 0, 1, 2, 3 };
    SortedIndices.Sort([&](int32 A, int32 B) { return Areas[A] < Areas[B]; });

    // 제일 작은 + 두 번째로 큰 (즉, 전체면적 기준 1등/3등)
    int32 SmallestIdx = SortedIndices[0];
    int32 SecondLargestIdx = SortedIndices[2];

    auto& Selected1 = Offsets[SmallestIdx];
    auto& Selected2 = Offsets[SecondLargestIdx];

    TArray<FIntPoint> GreenHouseOffset = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };

    for (int32 x = Selected1.Key.X; x <= Selected1.Value.X; x++) {
        for (int32 y = Selected1.Key.Y; y <= Selected1.Value.Y; y++) {
            FIntPoint Pos(x, y);
            if (ZoneMap.Contains(Pos)) continue;
            FGridCellData& Cell = ZoneMap.FindOrAdd(Pos);
            Cell.bIsCrossroad = false;
            Cell.ZoneType = EZoneType::Farmland;
        }
    }
    for (int32 x = Selected2.Key.X; x <= Selected2.Value.X; x++) {
        for (int32 y = Selected2.Key.Y; y <= Selected2.Value.Y; y++) {
            FIntPoint Pos(x, y);
            if (ZoneMap.Contains(Pos)) continue;
            FGridCellData& Cell = ZoneMap.FindOrAdd(Pos);
            Cell.bIsCrossroad = false;
            Cell.ZoneType = EZoneType::Farmland;
        }
    }
    // 비닐하우스 지정
    PlaceGreenhouse(Selected2);                 
    if (FMath::FRand() < 0.5f)            
    {
        PlaceGreenhouse(Selected1);
    }
    

    // 2. 도로 주변 인도 추가, 경작지면 경사로 추가
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
                    if (ZoneMap.Contains(MaybeCrossroad) && 
                        ZoneMap[MaybeCrossroad].bIsCrossroad && 
                        ZoneMap[MaybeCrossroad].ZoneType != EZoneType::Farmland /* && ZoneMap[MaybeCrossroad].CrossroadSize > 1*/)
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
            // 경작지 경사로 지정
            else if (ZoneMap.Contains(SidePos) && ZoneMap[SidePos].ZoneType == EZoneType::Farmland)
            {
                // 교차로 꼭짓점 체크
                bool bIsTrafficCorner = false;
                float TrafficYaw = 0.f;
                for (const FIntPoint& CornerOffset : CornerOffsetList)
                {
                    FIntPoint MaybeCrossroad = SidePos + CornerOffset;
                    if (ZoneMap.Contains(MaybeCrossroad) &&
                        ZoneMap[MaybeCrossroad].bIsCrossroad &&
                        ZoneMap[MaybeCrossroad].ZoneType != EZoneType::Farmland /* && ZoneMap[MaybeCrossroad].CrossroadSize > 1*/)
                    {
                        bIsTrafficCorner = true;
                        if (CornerYawMap.Contains(CornerOffset))
                        {
                            TrafficYaw = CornerYawMap[CornerOffset];
                        }
                        break;
                    }
                }

                // 경사로, 경사로 코너 구분
                FGridCellData& Cell = ZoneMap[SidePos];
                if (bIsTrafficCorner)
                {
                    Cell.ZoneType = EZoneType::FarmSlopeCorner;
                    Cell.PreferredRotation = FRotator(0.f, TrafficYaw, 0.f);
                }
                else
                {
                    Cell.ZoneType = EZoneType::FarmSlope;
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
        bool bDown = ZoneMap.Contains(Pos + FIntPoint(0, -1)) && (ZoneMap[Pos + FIntPoint(0, -1)].ZoneType == EZoneType::Road || ZoneMap[Pos + FIntPoint(0, -1)].ZoneType == EZoneType::Road_Crosswalk);

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
    *   1. 전체 ZoneMap 순회 -> EZoneType::Road인 셀만 순회
    *   2. 4방향 셀 조회 -> ERoadDirection::Crossroad 라면 현재 그리드 ZoneType을 Road_Crosswalk로 변경
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


    // 4. 건물 스폰 구역 지정
    /*
    *   1. 도로주변 상점 우선 배치 (상한선 있음)
    *   2. 민가 크기 2x2 ~ 5x5 (5x5는 넘 큰가?)
    *   3. 방향을 어디 기준으로 잡을지.. 아무튼 서로 마주보아야 함 (도로를 바라보지 않도록)
    */

    // MaxShopCount까지 상점가(+고층 빌딩) 배치
    int32 ShopCount = 0;
    
    for (int32 X = 0; X < GridWidth && !bShopLimitReached; ++X)
    {
        for (int32 Y = 0; Y < GridHeight && !bShopLimitReached; ++Y)
        {
            if (ShopCount >= MaxShopCount)
            {
                bShopLimitReached = true;
                break;
            }

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
            int32 FiveByFiveShopCount = 0;
            int32 S1 = FMath::RandRange(3, 5);

            if (S1 == 5) FiveByFiveShopCount++;

            while (FiveByFiveShopCount > MaxFiveByFiveShopCount && S1 == 5)
            {
                S1 = FMath::RandRange(3, 5);
            }
            //int32 S2 = RandomStream.RandRange(3, 3);

            // 3) 회전 랜덤 선택(인접 인도가 2방향 이상인 경우)
            int32 RotIdx = FMath::RandRange(0, ValidRotations.Num() - 1);
            FRotator Rotation = ValidRotations[RotIdx];

            // 4) 회전에 따른 가로/세로 결정
            bool bRotated = FMath::Abs(Rotation.Yaw) == 90.f;
            int32 Width = bRotated ? S1 : S1;
            int32 Height = bRotated ? S1 : S1;

            // 5) Top-Left 그리드 좌표 계산
            FIntPoint ShopTopLeft = GetTopLeftFromOrigin(Pos, Width, Height, Rotation);

            // 6) 영역 사용 가능 여부 검사
            if (!IsAreaAvailable(ShopTopLeft, Width, Height, /*BlockedTypes=*/{}))
                continue;

            // 7) 마킹 및 스폰 리스트에 추가
            if (!bShopLimitReached)
            {
                if (Width == 3)
                {
                    MarkZone(ShopTopLeft, Width, Height, EZoneType::Shop3, Rotation);
                    AddtoBuildingSpawnList(ShopTopLeft, Width, Height, EZoneType::Shop3, Rotation);
                }
                else if (Width == 4)
                {
                    MarkZone(ShopTopLeft, Width, Height, EZoneType::Shop4, Rotation);
                    AddtoBuildingSpawnList(ShopTopLeft, Width, Height, EZoneType::Shop4, Rotation);
                }
                else if (Width == 5)
                {
                    MarkZone(ShopTopLeft, Width, Height, EZoneType::Shop5, Rotation);
                    AddtoBuildingSpawnList(ShopTopLeft, Width, Height, EZoneType::Shop5, Rotation);
                }
                ShopCount++;
                if (ShopCount >= MaxShopCount)
                {
                    bShopLimitReached = true;
                    break;
                }
            }
            
            

        }
    }


    // 5. 남은 빈 공간에 주택 배치
    /*
    *   주택 배치 알고리즘
    *   
    */
    int32 HouseCount = 0;
    for (int32 X = 0; X < GridWidth && !bHouseLimitReached; ++X)
    {
        for (int32 Y = 0; Y < GridHeight && !bHouseLimitReached; ++Y)
        {
            if (HouseCount >= MaxHouseCount) 
            {
                bHouseLimitReached = true;
                break;
            }

            FIntPoint Pos(X, Y);
            if (ZoneMap.Contains(Pos)) continue;  // 이미 마킹된 셀 건너뛰기

            // 0 우 / 1 좌 / 2 상 / 3 하
            for (int32 Index = 0; Index < SearchOffsetList.Num(); Index++)
            {
                FIntPoint N = Pos + SearchOffsetList[Index];
                if (ZoneMap.Contains(N) && ZoneMap[N].ZoneType == EZoneType::Road_Sidewalk)
                {
                    /*
                    *   도로가 우측에 있음 -> 좌측 방향으로 진행
                    *   건물은 위 아래로 마주보아야 함
                    */
                    if (Index == 0)
                    {
                        UE_LOG(LogTemp, Display, TEXT("도로가 우측 -> 좌측 방향으로 진행(X-)"));

                        // 포인터 초기화
                        FIntPoint Pos1 = Pos;
                        FIntPoint Pos2 = Pos;

                        // 경계에 닿을 때까지 계속 진행
                        while (Pos1.X >= 0 && Pos1.X < GridWidth && Pos1.Y >= 0 && Pos1.Y < GridHeight
                            && Pos2.X >= 0 && Pos2.X < GridWidth && Pos2.Y >= 0 && Pos2.Y < GridHeight)
                        {
                            // 집 크기 결정
                            int32 Size1 = FMath::RandRange(3, 5);
                            int32 Size2 = FMath::RandRange(3, 5);

                            // 포인터 설정
                            // Pos1은 그 자리에서 X만 변하면 됨               
                            Pos2 = FIntPoint(Pos1.X, Pos1.Y - Size1);
                            bool bIsPos2Up = false;
                            // 위 아래 공간없으면 패스
                            if (Pos2.Y < 0 || ZoneMap.Contains(Pos2))
                            {
                                Pos2 = FIntPoint(Pos1.X, Pos1.Y + Size1);
                                bIsPos2Up = true;
                                if (Pos2.Y >= GridHeight || ZoneMap.Contains(Pos2))
                                {
                                    bIsPos2Up = false;
                                }
                            }
                            // 포인터1 집 설치
                            FRotator Rot1 = bIsPos2Up ? FRotator(0, 0, 0) : FRotator(0, 180, 0);
                            FIntPoint TL1 = GetTopLeftFromOrigin(Pos1, Size1, Size1, Rot1);

                            UE_LOG(LogTemp, Display, TEXT("Pos1 : (%d, %d), TL1 : (%d, %d), Size1 : %d"),Pos1.X, Pos1.Y, TL1.X, TL1.Y, Size1);

                            if (!bHouseLimitReached && IsAreaAvailable(TL1, Size1, Size1, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);
                                AddtoBuildingSpawnList(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }
                            
                            // 포인터2 집 설치

                            FRotator Rot2 = bIsPos2Up ? FRotator(0, 180, 0) : FRotator(0, 0, 0);
                            FIntPoint TL2 = FIntPoint(TL1.X, TL1.Y + (bIsPos2Up ? Size1 : -(Size1)));
                            if (!IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {

                                UE_LOG(LogTemp, Display, TEXT("Area not available : Pos2 : (%d, %d), TopLeft : (%d, %d) Size : %d "),Pos2.X, Pos2.Y, TL2.X, TL2.Y, Size2);
                            }
                            if (!bHouseLimitReached && IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);
                                AddtoBuildingSpawnList(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);
                                
                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }

                            Pos1.X -= 1;
                            Pos2.X -= 1;
                        }
                    }

                    /*
                    *   도로가 좌측에 있음 -> 우측으로 진행
                    *   건물 위 아래 마주보기
                    */
                    else if (Index == 1)
                    {
                        UE_LOG(LogTemp, Display, TEXT("도로가 좌측에 있음 -> 우측으로 건물 진행 (X+)"));
                        
                        // 포인터 초기화
                        FIntPoint Pos1 = Pos;
                        FIntPoint Pos2 = Pos;

                        // 경계에 닿을 때까지 계속 진행
                        while (Pos1.X >= 0 && Pos1.X < GridWidth && Pos1.Y >= 0 && Pos1.Y < GridHeight
                            && Pos2.X >= 0 && Pos2.X < GridWidth && Pos2.Y >= 0 && Pos2.Y < GridHeight)
                        {
                            // 집 크기 결정
                            int32 Size1 = FMath::RandRange(3, 5);
                            int32 Size2 = FMath::RandRange(3, 5);

                            // 포인터 설정
                            // Pos1은 그 자리에서 X만 변하면 됨               
                            Pos2 = FIntPoint(Pos1.X, Pos1.Y - Size1);
                            bool bIsPos2Up = false;
                            // 위 아래 공간없으면 패스
                            if (Pos2.Y < 0 || ZoneMap.Contains(Pos2))
                            {
                                Pos2 = FIntPoint(Pos1.X, Pos1.Y + Size1);
                                bIsPos2Up = true;
                                if (Pos2.Y >= GridHeight || ZoneMap.Contains(Pos2))
                                {
                                    bIsPos2Up = false;
                                }
                            }
                            // 포인터1 집 설치
                            FRotator Rot1 = bIsPos2Up ? FRotator(0, 0, 0) : FRotator(0, 180, 0);
                            FIntPoint TL1 = GetTopLeftFromOrigin(Pos1, Size1, Size1, Rot1);

                            UE_LOG(LogTemp, Display, TEXT("Pos1 : (%d, %d), TL1 : (%d, %d), Size1 : %d"), Pos1.X, Pos1.Y, TL1.X, TL1.Y, Size1);

                            if (!bHouseLimitReached && IsAreaAvailable(TL1, Size1, Size1, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);
                                AddtoBuildingSpawnList(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }

                            // 포인터2 집 설치

                            FRotator Rot2 = bIsPos2Up ? FRotator(0, 180, 0) : FRotator(0, 0, 0);
                            FIntPoint TL2 = FIntPoint(TL1.X, TL1.Y + (bIsPos2Up ? Size1 : -(Size1)));
                            if (!IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                UE_LOG(LogTemp, Display, TEXT("Area not available : Pos2 : (%d, %d), TopLeft : (%d, %d) Size : %d "), Pos2.X, Pos2.Y, TL2.X, TL2.Y, Size2);
                            }
                            if (!bHouseLimitReached && IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);
                                AddtoBuildingSpawnList(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }

                            Pos1.X += 1;
                            Pos2.X += 1;
                        }
                    }

                    /*
                    *   도로가 위에 있음 -> 아래로 진행
                    *   주택은 좌우로 마주보게 설정
                    */
                    else if (Index == 2)
                    {
                        UE_LOG(LogTemp, Display, TEXT("도로 위에 있음 -> 건물 아래로 진행 (-Y)"));
                        // 포인터 초기화
                        FIntPoint Pos1 = Pos;
                        FIntPoint Pos2 = FIntPoint(Pos1.X + 1, Pos1.Y);

                        // 경계에 닿을 때까지 계속 진행
                        while (Pos1.X >= 0 && Pos1.X < GridWidth && Pos1.Y >= 0 && Pos1.Y < GridHeight
                            && Pos2.X >= 0 && Pos2.X < GridWidth && Pos2.Y >= 0 && Pos2.Y < GridHeight)
                        {
                            // 집 크기 결정
                            int32 Size1 = FMath::RandRange(3, 5);
                            int32 Size2 = FMath::RandRange(3, 5);

                            // 포인터 설정
                            // Pos1은 그 자리에서 Y만 변하면 됨               
                            Pos2 = FIntPoint(Pos1.X - Size1, Pos1.Y);
                            bool bIsPos2Right = false;
                            // 위 아래 공간없으면 패스
                            if (Pos2.Y < 0 || ZoneMap.Contains(Pos2))
                            {
                                Pos2 = FIntPoint(Pos1.X + Size1, Pos1.Y);
                                bIsPos2Right = true;
                                if (Pos2.Y >= GridHeight || ZoneMap.Contains(Pos2))
                                {
                                    bIsPos2Right = false;
                                }
                            }
                            // 포인터1 집 설치
                            FRotator Rot1 = bIsPos2Right ? FRotator(0, 90.f, 0) : FRotator(0, -90.f, 0);
                            FIntPoint TL1 = GetTopLeftFromOrigin(Pos1, Size1, Size1, Rot1);

                            UE_LOG(LogTemp, Display, TEXT("Pos1 : (%d, %d), TL1 : (%d, %d), Size1 : %d"), Pos1.X, Pos1.Y, TL1.X, TL1.Y, Size1);

                            if (!bHouseLimitReached && IsAreaAvailable(TL1, Size1, Size1, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);
                                AddtoBuildingSpawnList(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }
                            
                            // 포인터2 집 설치
                            FRotator Rot2 = bIsPos2Right ? FRotator(0, -90.f, 0) : FRotator(0, 90.f, 0);
                            FIntPoint TL2 = FIntPoint(TL1.X + (bIsPos2Right ? Size1 : -(Size1)), TL1.Y);
                            if (!IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                UE_LOG(LogTemp, Display, TEXT("Area not available : Pos2 : (%d, %d), TopLeft : (%d, %d) Size : %d "), Pos2.X, Pos2.Y, TL2.X, TL2.Y, Size2);
                            }
                            if (!bHouseLimitReached && IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);
                                AddtoBuildingSpawnList(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }
                            
                            Pos1.Y -= 1;
                            Pos2.Y -= 1;
                        }
                    }
                    /*
                    *   도로가 아래에 있음 -> 위쪽으로 진행
                    */
                    
                    else if (Index == 3)
                    {
                        UE_LOG(LogTemp, Display, TEXT("도로 위에 있음 -> 건물 아래로 진행 (-Y)"));
                        // 포인터 초기화
                        FIntPoint Pos1 = Pos;
                        FIntPoint Pos2 = FIntPoint(Pos1.X + 1, Pos1.Y);

                        // 경계에 닿을 때까지 계속 진행
                        while (Pos1.X >= 0 && Pos1.X < GridWidth && Pos1.Y >= 0 && Pos1.Y < GridHeight
                            && Pos2.X >= 0 && Pos2.X < GridWidth && Pos2.Y >= 0 && Pos2.Y < GridHeight)
                        {
                            // 집 크기 결정
                            int32 Size1 = FMath::RandRange(3, 5);
                            int32 Size2 = FMath::RandRange(3, 5);

                            // 포인터 설정
                            // Pos1은 그 자리에서 Y만 변하면 됨               
                            Pos2 = FIntPoint(Pos1.X - Size1, Pos1.Y);
                            bool bIsPos2Right = false;
                            // 위 아래 공간없으면 패스
                            if (Pos2.Y < 0 || ZoneMap.Contains(Pos2))
                            {
                                Pos2 = FIntPoint(Pos1.X + Size1, Pos1.Y);
                                bIsPos2Right = true;
                                if (Pos2.Y >= GridHeight || ZoneMap.Contains(Pos2))
                                {
                                    bIsPos2Right = false;
                                }
                            }
                            // 포인터1 집 설치
                            FRotator Rot1 = bIsPos2Right ? FRotator(0, 90.f, 0) : FRotator(0, -90.f, 0);
                            FIntPoint TL1 = GetTopLeftFromOrigin(Pos1, Size1, Size1, Rot1);

                            UE_LOG(LogTemp, Display, TEXT("Pos1 : (%d, %d), TL1 : (%d, %d), Size1 : %d"), Pos1.X, Pos1.Y, TL1.X, TL1.Y, Size1);

                            if (!bHouseLimitReached && IsAreaAvailable(TL1, Size1, Size1, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);
                                AddtoBuildingSpawnList(TL1, Size1, Size1,
                                    (Size1 == 3 ? EZoneType::House3 : (Size1 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot1);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }

                            // 포인터2 집 설치
                            FRotator Rot2 = bIsPos2Right ? FRotator(0, -90.f, 0) : FRotator(0, 90.f, 0);
                            FIntPoint TL2 = FIntPoint(TL1.X + (bIsPos2Right ? Size1 : -(Size1)), TL1.Y);
                            if (!IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                UE_LOG(LogTemp, Display, TEXT("Area not available : Pos2 : (%d, %d), TopLeft : (%d, %d) Size : %d "), Pos2.X, Pos2.Y, TL2.X, TL2.Y, Size2);
                            }
                            if (!bHouseLimitReached && IsAreaAvailable(TL2, Size2, Size2, /*BlockedTypes*/{}))
                            {
                                MarkZone(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);
                                AddtoBuildingSpawnList(TL2, Size2, Size2,
                                    (Size2 == 3 ? EZoneType::House3 : (Size2 == 4 ? EZoneType::House4 : EZoneType::House5)), Rot2);

                                HouseCount++;
                                if (HouseCount >= MaxHouseCount)
                                {
                                    bHouseLimitReached = true;
                                    break;
                                }
                            }

                            Pos1.Y += 1;
                            Pos2.Y += 1;
                        }
                    }

                }

            }            
        }
    }

    // 5. 특수 부지 지정
    AssignSpecialClusters();

    // 6. 남은 빈 칸에 골목, 쓰레기장 스폰
    /*
    *   1. 사방 중 집이 있으면 골목으로 설정
    *   2. 사방 중 도로가 있으면 쓰레기장 설정
    */
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
                Cell.ZoneType = EZoneType::Garbage;
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

void ARuralGenerator::TrySpawnBorder(const FVector& Location, const FIntPoint& GridPos)
{
    // AABB
    FBox QueryBox = FBox::BuildAABB(Location, FVector(100.f, 100.f, 100.f));
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        // 프리팹 감지 실패 -> 스킵
        if (!Actor || !QueryBox.IsInside(Actor->GetActorLocation())) continue;

        const TArray<TPair<FString, FIntPoint>> ArrowOffset =
        {
            {TEXT("Left"),    FIntPoint(0, 1)},
            {TEXT("Right"),  FIntPoint(0, -1)},
            {TEXT("Up"),  FIntPoint(-1, 0)},
            {TEXT("Down"), FIntPoint(1, 0)},
        };

        for (const auto& Pair : ArrowOffset)
        {
            FString ArrowName = Pair.Key.ToLower();
            FIntPoint Direction = Pair.Value;
            FIntPoint SearchPos = GridPos + Direction;

            // 인도 검사 -> 펜스 스폰
            if (ZoneMap.Contains(SearchPos) &&
                (ZoneMap[SearchPos].ZoneType == EZoneType::Shop3 ||
                    ZoneMap[SearchPos].ZoneType == EZoneType::Shop4 ||
                    ZoneMap[SearchPos].ZoneType == EZoneType::Shop5 ||
                    ZoneMap[SearchPos].ZoneType == EZoneType::House3 ||
                    ZoneMap[SearchPos].ZoneType == EZoneType::House5))
            {

                TArray<USceneComponent*> Components;
                Actor->GetComponents<USceneComponent>(Components);

                for (USceneComponent* Comp : Components)
                {
                    if (!Comp) continue;

                    FString Name = Comp->GetName().ToLower();
                    if (Name.Contains(ArrowName))
                    {
                        FTransform SpawnTransform = Comp->GetComponentTransform();

                        AActor* SpawnedFence = nullptr;
                        if (FencePrefab && FMath::FRand() < AlleyLightSpawnChance)
                        {
                            SpawnedFence = GetWorld()->SpawnActor<AActor>(FencePrefab, SpawnTransform);
                            if (SpawnedFence)
                            {
                                FVector CompScale = Comp->GetComponentScale();
                                if (SpawnedFence->GetRootComponent())
                                {
                                    SpawnedFence->GetRootComponent()->SetWorldScale3D(CompScale);
                                }
                            }

                        }
                        //break;
                    }
                }

            }
        }

    }
}

void ARuralGenerator::PlaceGreenhouse(const TPair<FIntPoint, FIntPoint>& Range)
{
    TArray<FIntPoint> Candidates;

    for (int32 x = Range.Key.X +1; x <= Range.Value.X - 1; ++x)
    {
        for (int32 y = Range.Key.Y + 1; y <= Range.Value.Y - 1; ++y)
        {
            // 4×4 블록의 TopLeft
            FIntPoint TL(x, y);            

            bool bIsAllFarmland = true;
            for (int dx = 0; dx < 4 && bIsAllFarmland; ++dx)
            {
                for (int dy = 0; dy < 4; ++dy)
                {
                    FIntPoint P = TL + FIntPoint(dx, dy);

                    // 만약 Farmland가 아니면 후보 탈락
                    if (!ZoneMap.Contains(P) ||
                        ZoneMap[P].ZoneType != EZoneType::Farmland)
                    {
                        bIsAllFarmland = false;
                        break;
                    }
                }
            }

            if (bIsAllFarmland)
            {
                Candidates.Add(TL);
            }
        }
    }

    if (Candidates.Num() == 0) return;          // 자리 없음

    // 랜덤으로 하나 뽑기
    int32 Pick = FMath::RandRange(0, Candidates.Num() - 1);
    FIntPoint TL = Candidates[Pick];
    FRotator   Rot = FRotator::ZeroRotator;

    // Greenhouse 로 덮어쓰기
    MarkZone(TL, 4, 4, EZoneType::Greenhouse, Rot);
    AddtoBuildingSpawnList(TL, 4, 4, EZoneType::Greenhouse, Rot);
}
