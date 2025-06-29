#include "MapGenerator/GlobalDebrisGenerator.h"
#include "MapGenerator/MapGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


AGlobalDebrisGenerator::AGlobalDebrisGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    
    Scale = FVector(1.f, 1.f, 1.f);

    // 초기 Dormancy
    SetNetDormancy(DORM_Awake);

    // 기본 시드 (에디터에서 조정 가능)
    Seed = FMath::RandRange(1, 999999);


}

void AGlobalDebrisGenerator::StartGenerateDebris()
{
    // 타일 정보 수집
    GatherTiles();
    if (TileInfos.Num() == 0 || MeshVariants.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] No tiles or no mesh variants."), *GetName());
        return;
    }

    // 서버 권한일 때만 실제 스폰 로직 실행
    if (HasAuthority())
    {
        // 랜덤 스트림 초기화 (서버 전용)
        RandomStream.Initialize(Seed);
        SpawnAllDebris();

    }
}

void AGlobalDebrisGenerator::BeginPlay()
{
    Super::BeginPlay();

    // 메시 종류별 HISM 컴포넌트 생성
    CreateHISMComponents();

    // 맵 생성기 참조
    MapGenerator = Cast<AMapGenerator>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMapGenerator::StaticClass()));
    if (!MapGenerator)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] MapGenerator not found."), *GetName());
        return;
    }

    if (HasAuthority())
    {
        SetNetDormancy(DORM_Awake);
    }
}

void AGlobalDebrisGenerator::Multicast_SpawnDebrisBatch_Implementation(const TArray<FGlobalDebrisSpawnData>& InBatch)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] Multicast received %d items (NetMode=%d)"),
        *GetName(),
        InBatch.Num(),
        (int)GetNetMode());
    
    for (auto& D : InBatch) SpawnLocalInstance(D);

}

void AGlobalDebrisGenerator::SpawnLocalInstance(const FGlobalDebrisSpawnData& D)
{
    if (HISMComponents.IsValidIndex(D.MeshIndex))
    {
        auto* H = HISMComponents[D.MeshIndex];

        //FVector LocalPos = H->GetComponentTransform().InverseTransformPosition(D.Location);

        FTransform InstTM(FRotator(0.f, D.RotationYaw, 0.f), D.Location, D.Scale);

        H->AddInstance(InstTM, true);
        H->MarkRenderStateDirty();
        UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnLocalInstance on mesh %d → total %d | NetMode=%d, Role=%d"), *GetName(), D.MeshIndex,H->GetInstanceCount(),(int)GetNetMode(), (int)GetLocalRole());
    }
}

void AGlobalDebrisGenerator::Multicast_FinalizeSpawn_Implementation()
{
    SetNetDormancy(DORM_DormantAll);
    UE_LOG(LogTemp, Log, TEXT("[%s] Finalized spawning and set to dormant."), *GetName());

    OnDebrisSpawnComplete.Broadcast();
}


// 타일 정보 수집: ZoneMap을 순회해 TargetZoneType인 타일만
void AGlobalDebrisGenerator::GatherTiles()
{
    TileInfos.Empty();

    for (const auto& Pair : MapGenerator->ZoneMap)
    {
        const FGridCellData& Zone = Pair.Value;
        if (Zone.ZoneType == TargetZoneType)
        {
            // 그리드 좌표 월드 위치 변환 
            FVector Center(Pair.Key.X * 500.f, Pair.Key.Y * 500.f, 0.f);
            FVector Extent(225.f, 225.f, 50.f);
            if (bUseInnerTile)
            {
                Extent = FVector(5.f, 5.f, 50.f);
            }

            TileInfos.Add({ Center, Extent });
        }
    }
}

void AGlobalDebrisGenerator::CreateHISMComponents()
{
    HISMComponents.Empty();

    for (int32 i = 0; i < MeshVariants.Num(); ++i)
    {
        UStaticMesh* Mesh = MeshVariants[i];
        if (!Mesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("MeshVariants failed"));
            continue;
        }

        // 컴포넌트 생성 & 세팅
        FString CompName = FString::Printf(TEXT("HISM_%d"), i);
        UHierarchicalInstancedStaticMeshComponent* HISM =
            NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *CompName);


        HISM->SetStaticMesh(Mesh);
        HISM->SetMobility(EComponentMobility::Movable);
        HISM->SetCollisionEnabled(bShouldCheckCollision ? ECollisionEnabled::QueryAndPhysics
            : ECollisionEnabled::NoCollision);
        HISM->InstanceStartCullDistance = 8000.f;
        HISM->InstanceEndCullDistance = 9000.f;

        HISM->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        HISM->SetRelativeTransform(FTransform::Identity);
        HISM->RegisterComponent();

        HISMComponents.Add(HISM);

        UE_LOG(LogTemp, Warning, TEXT("[%s][%s] Registered HISM component %s at index %d (Ptr=%p), TotalHISMs now %d"),
            *GetName(),
            HasAuthority() ? TEXT("Server") : TEXT("Client"),
            *CompName,
            i,
            HISM,
            HISMComponents.Num() + 1);

    }
}

void AGlobalDebrisGenerator::SpawnAllDebris()
{


    UWorld* World = GetWorld();
    if (!World) return;

    // 충돌 검사용 배열
    TArray<FVector> PlacedLocations;
    TArray<float>  PlacedRadii;

    // 메시 선택 & 타일별 뿌리기
    for (int32 Iter = 0; Iter < Iterations; ++Iter)
    {
        // 랜덤으로 메시 종류 선택
        int32 MeshIndex = RandomStream.RandRange(0, MeshVariants.Num() - 1);
        UHierarchicalInstancedStaticMeshComponent* HISM = HISMComponents[MeshIndex];
        UStaticMesh* ChosenMesh = MeshVariants[MeshIndex];
        if (!ChosenMesh) continue;

        // 인스턴스 반경 계산
        float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();

        for (const FTileInfo& Tile : TileInfos)
        {
            for (int32 n = 0; n < NumPerTile; ++n)
            {
                if (FMath::FRand() > SpawnChance) continue;

                // 타일 영역 내 랜덤 위치
                FVector Off(
                    RandomStream.FRandRange(-Tile.Extent.X, Tile.Extent.X),
                    RandomStream.FRandRange(-Tile.Extent.Y, Tile.Extent.Y),
                    0.f
                );
                FVector Start = Tile.Center + Off + FVector(0, 0, 500.f);
                FVector End = Tile.Center + Off + FVector(0, 0, -500.f);

                // 라인트레이스(지면 충돌 체크)
                FHitResult Hit;
                FCollisionQueryParams Params;
                Params.AddIgnoredActor(this);
                if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
                {
                    continue;
                }

                FVector Loc = Hit.ImpactPoint;
                FRotator Rot = FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f);
                if (bUseInnerTile)
                {
                    float RandomYaw = FMath::RandRange(0, 3) * 90.0f;
                    Rot = FRotator(0.f, RandomYaw, 0.f);
                }
                uint16 Yaw = Rot.Yaw;
                FVector Scl = Scale;

                // 중복 충돌 검사
                bool bBlocked = false;
                if (bShouldCheckCollision)
                {
                    for (int32 i = 0; i < PlacedLocations.Num(); ++i)
                    {
                        float MinDist = InstanceRadius + PlacedRadii[i];
                        if (FVector::DistSquared(PlacedLocations[i], Loc) < FMath::Square(MinDist))
                        {
                            bBlocked = true;
                            break;
                        }
                    }
                }
                if (bBlocked)
                {
                    // 너무 가까운 위치: 스킵
                    continue;
                }

                // 충돌 안 날 때만 배열에 기록
                PlacedLocations.Add(Loc);
                PlacedRadii.Add(InstanceRadius);

                FGlobalDebrisSpawnData NewItem;
                NewItem.Location = Loc;
                NewItem.RotationYaw = Yaw;
                NewItem.Scale = Scl;
                NewItem.MeshIndex = MeshIndex;

                Batch.Add(NewItem);
            }
        }
    }

    // 서버가 멀티 캐스트로 뿌림
    if (HasAuthority())
    {
        PendingBatchStart = 0;

        GetWorld()->GetTimerManager().SetTimer(
            BatchTimerHandle,
            this,
            &AGlobalDebrisGenerator::SendNextChunk,
            0.05f,
            true /* 함수에서 타이머 정지 */
        );
    }
}

void AGlobalDebrisGenerator::SendNextChunk()
{
    // Batch를 ChunkSize씩 잘라서 순차적 Multicast 호출
    const int32 Total = Batch.Num();
    if (PendingBatchStart >= Total)
    {
        GetWorld()->GetTimerManager().ClearTimer(BatchTimerHandle);
        Multicast_FinalizeSpawn();
        return;
    }

    
    int32 Count = FMath::Min(RPCChunkSize, Total - PendingBatchStart);
    TArray<FGlobalDebrisSpawnData> SubBatch;
    SubBatch.Append(&Batch[PendingBatchStart], Count);

    Multicast_SpawnDebrisBatch(SubBatch);

    PendingBatchStart += Count;
}