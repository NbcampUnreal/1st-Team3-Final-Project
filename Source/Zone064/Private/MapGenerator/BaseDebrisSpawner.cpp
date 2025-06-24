#include "MapGenerator/BaseDebrisSpawner.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "MapGenerator/MapGenerator.h"
#include "MapGenerator/CityBlockBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Serialization/FastArraySerializer.h"

ABaseDebrisSpawner::ABaseDebrisSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    RootComponent = SpawnArea;
    SpawnArea->SetBoxExtent(FVector(250.f, 250.f, 50.f));
    SpawnArea->SetHiddenInGame(true);
    SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);


    NumInstances = 10;
    SpawnChance = 1.0f;
    VehicleSpawnRatio = 0.1f;
    VehicleSpawnChance = 0.2f;
    bUseSplitMeshArrays = false;

    bShouldCheckCollision = true;
    Scale = FVector(1.f, 1.f, 1.f);

    DebrisArray.Spawner = this;
    SetNetDormancy(ENetDormancy::DORM_Initial);
}

void ABaseDebrisSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseDebrisSpawner, DebrisArray);
}

void ABaseDebrisSpawner::BeginPlay()
{
    Super::BeginPlay();

    MapGenerator = Cast<AMapGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapGenerator::StaticClass()));

    DebrisArray.Spawner = this;

    RandomStream.Initialize(Seed);
    
    if (HasAuthority())
    {
        GenerateInstances();
    }
    UE_LOG(LogTemp, Warning, TEXT("### NumInstances = %d (HasAuthority=%d)"), NumInstances, HasAuthority());
}

void ABaseDebrisSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseDebrisSpawner::GenerateInstances()
{
    int32 TraceMiss = 0, NoMesh = 0;
    
    if (!HasAuthority()) return;

    DebrisArray.Instances.Empty();
    DebrisArray.MarkArrayDirty();

    // 교차로에선 스폰 안함
    if (CheckCrossRoad()) return;

    for (auto& Pair : MeshToComponentMap)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearInstances();
        }
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FTransform SpawnTransform = SpawnArea->GetComponentTransform();
    FVector Extent = SpawnArea->GetScaledBoxExtent();

    TArray<FVector> PlacedLocations;
    TArray<float> PlacedRadii;

    int32 SpawnedCount = 0;
    int32 MaxAttempts = NumInstances * 5;
    int32 Attempt = 0;

    // 통합 랜덤 방식
    if (!bUseSplitMeshArrays)
    {
        if (MeshVariants.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("MeshVariants 배열이 비어 있습니다."));
            return;
        }

        for (; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
        {
            
            FVector LocalRandom = FVector(
                RandomStream.FRandRange(-Extent.X, Extent.X),
                RandomStream.FRandRange(-Extent.Y, Extent.Y),
                0.f
            );

            // 지면 라인트레이스
            FVector Start = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, 500.f));
            FVector End = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, -1000.f));

            FHitResult HitResult;
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(this);

            if (!World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams))
            {
                TraceMiss++;
                continue;
            }
                

            int32 Index = FMath::RandRange(0, MeshVariants.Num() - 1);
            UStaticMesh* ChosenMesh = MeshVariants.IsValidIndex(Index) ? MeshVariants[Index] : nullptr;
            if (!ChosenMesh)
            {
                NoMesh++;
                continue;
            }

            float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();
            FVector SpawnLocation = HitResult.ImpactPoint;
            FRotator RandomRot(0, FMath::FRandRange(0.f, 360.f), 0);

            // 중복 충돌 검사
            bool bBlocked = false;
            if (bShouldCheckCollision)
            {
                for (int32 i = 0; i < PlacedLocations.Num(); ++i)
                {
                    float MinDist = InstanceRadius + PlacedRadii[i];
                    if (FVector::DistSquared(PlacedLocations[i], SpawnLocation) < FMath::Square(MinDist))
                    {
                        bBlocked = true;
                        break;
                    }
                }
            }

            float CollisionCheckRoll = RandomStream.FRand();
            if (bBlocked)
            {
                UE_LOG(LogTemp, Warning, TEXT("Blocked: %s at %s"), *ChosenMesh->GetName(), *SpawnLocation.ToString());
                continue;
            }

            UHierarchicalInstancedStaticMeshComponent* MeshComp = GetOrCreateInstancedMeshComponent(ChosenMesh);
            if (!MeshComp) continue;

            FVector LocalPos = MeshComp->GetComponentTransform().InverseTransformPosition(SpawnLocation);
            FTransform InstanceTransform(RandomRot, LocalPos, Scale);
            
            // 클라이언트에 전달할 구조체 저장
            FDebrisInstanceDataItem NewItem;
            NewItem.Location = SpawnLocation;
            NewItem.Rotation = RandomRot;
            NewItem.MeshIndex = Index;
            NewItem.Scale = Scale;
            NewItem.MeshType = EDebrisMeshType::Variant;
            DebrisArray.Instances.Add(NewItem);
            DebrisArray.MarkItemDirty(NewItem);

            MeshComp->AddInstance(InstanceTransform);
            //DrawDebugBox(World, SpawnLocation, ChosenMesh->GetBounds().BoxExtent, FColor::Red, false, 30.0f, 0, 2.0f);

            PlacedLocations.Add(SpawnLocation);
            PlacedRadii.Add(InstanceRadius);
            SpawnedCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("[Simple] %d개의 인스턴스를 생성했습니다."), SpawnedCount);
        UE_LOG(LogTemp, Warning, TEXT("Spawned %d / Attempts %d (TraceMiss %d, NoMesh %d)"), SpawnedCount, Attempt, TraceMiss, NoMesh);
        return;
    }

    // 분리 방식 (차량 vs 기타)
    if (VehicleMeshes.Num() == 0 && OtherMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VehicleMeshes 또는 OtherMeshes 배열이 비어 있습니다."));
        return;
    }

    int32 VehicleLimit = FMath::RoundToInt(NumInstances * VehicleSpawnRatio);
    float VehicleRoll = FMath::FRand();
    if (VehicleRoll > VehicleSpawnChance) VehicleLimit = 0;

    int32 VehicleCount = 0;
    for (; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
    {
        float SpawnChanceRoll = FMath::FRand();
        if (SpawnChanceRoll > SpawnChance)
            continue;

        FDebrisInstanceDataItem NewItem;
        FVector LocalRandom = FVector(
            RandomStream.FRandRange(-Extent.X, Extent.X),
            RandomStream.FRandRange(-Extent.Y, Extent.Y),
            0.f
        );

        FVector Start = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, 500.f));
        FVector End = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, -1000.f));

        FHitResult HitResult;
        FCollisionQueryParams TraceParams;
        TraceParams.AddIgnoredActor(this);

        if (!World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams))
            continue;

        FVector SpawnLocation = HitResult.ImpactPoint;
        FRotator RandomRot(0, FMath::FRandRange(0.f, 360.f), 0);

        UStaticMesh* ChosenMesh = nullptr;
        bool bIsVehicle = false;

        float VehicleSpawnRoll = FMath::FRand();
        int32 Index = 0;
        if (VehicleCount < VehicleLimit && VehicleMeshes.Num() > 0 && VehicleSpawnRoll < VehicleSpawnRatio)
        {
            Index = RandomStream.RandRange(0, VehicleMeshes.Num() - 1);
            ChosenMesh = VehicleMeshes[Index];
            NewItem.MeshType = EDebrisMeshType::Vehicle;
            bIsVehicle = true;
        }
        else if (OtherMeshes.Num() > 0)
        {
            Index = RandomStream.RandRange(0, OtherMeshes.Num() - 1);
            ChosenMesh = OtherMeshes[Index];
            NewItem.MeshType = EDebrisMeshType::Other;
        }

        if (!ChosenMesh) continue;

        float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();

        // 중복 충돌 검사
        bool bBlocked = false;
        if (bShouldCheckCollision)
        {
            for (int32 i = 0; i < PlacedLocations.Num(); ++i)
            {
                float MinDist = InstanceRadius + PlacedRadii[i];
                if (FVector::DistSquared(PlacedLocations[i], SpawnLocation) < FMath::Square(MinDist))
                {
                    bBlocked = true;
                    break;
                }
            }
        }
        float CollisionCheckRoll = FMath::FRand();
        if (bBlocked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Blocked: %s at %s"), *ChosenMesh->GetName(), *SpawnLocation.ToString());
            continue;
        }

        UHierarchicalInstancedStaticMeshComponent* MeshComp = GetOrCreateInstancedMeshComponent(ChosenMesh);
        if (!MeshComp) continue;

        FVector LocalPos = MeshComp->GetComponentTransform().InverseTransformPosition(SpawnLocation);
        FTransform InstanceTransform(RandomRot, LocalPos, Scale);

        // 클라이언트에 전달할 구조체 저장 객체는 위에 있음
        NewItem.Location = SpawnLocation;
        NewItem.Rotation = RandomRot;
        NewItem.MeshIndex = Index;
        NewItem.Scale = Scale;
        DebrisArray.Instances.Add(NewItem);
        DebrisArray.MarkItemDirty(NewItem);

        MeshComp->AddInstance(InstanceTransform);
        //DrawDebugBox(World, SpawnLocation, ChosenMesh->GetBounds().BoxExtent, FColor::Blue, false, 30.0f, 0, 2.0f);

        PlacedLocations.Add(SpawnLocation);
        PlacedRadii.Add(InstanceRadius);
        if (bIsVehicle) VehicleCount++;
        SpawnedCount++;
    }
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d / Attempts %d (TraceMiss %d, NoMesh %d)"), SpawnedCount, Attempt, TraceMiss, NoMesh);
    UE_LOG(LogTemp, Log, TEXT("[Split] %d개의 인스턴스를 생성했습니다. (차량 %d개 / 제한 %d개)"), SpawnedCount, VehicleCount, VehicleLimit);

    
    // 복제 후 휴면 상태 설정
    SetNetDormancy(DORM_Awake);
    ForceNetUpdate();
    SetNetDormancy(DORM_DormantAll);
}



UHierarchicalInstancedStaticMeshComponent* ABaseDebrisSpawner::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh)
{
    if (!Mesh) return nullptr;

    if (MeshToComponentMap.Contains(Mesh))
    {
        return MeshToComponentMap[Mesh];
    }

    FString Name = FString::Printf(TEXT("HISMC_%s"), *Mesh->GetName());
    UHierarchicalInstancedStaticMeshComponent* NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *Name);
    check(NewComp);
    NewComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    NewComp->SetRelativeTransform(FTransform::Identity);
    NewComp->RegisterComponent();
    NewComp->SetStaticMesh(Mesh);
    NewComp->InstanceStartCullDistance = 7000.f;
    NewComp->InstanceEndCullDistance = 9000.f;
    if (bShouldCheckCollision)
    {
        NewComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        NewComp->SetCollisionResponseToAllChannels(ECR_Block);
    }
    else if (!bShouldCheckCollision)
    {
        NewComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    MeshToComponentMap.Add(Mesh, NewComp);
    return NewComp;
}

bool ABaseDebrisSpawner::CheckCrossRoad()
{
    
    // MapGenerator가 (0, 0, 0)이어야 함
    FVector SpawnerLocation = GetActorLocation();
    FIntPoint GridPos = FIntPoint(SpawnerLocation.X / 500.0, SpawnerLocation.Y / 500.0);
    
    bool bIsParentCrossroad = MapGenerator->ZoneMap[GridPos].bIsCrossroad;

    return bIsParentCrossroad;
}

void ABaseDebrisSpawner::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    FVector Location = GetActorLocation();
    uint32 PosHash = FCrc::MemCrc32(&Location, sizeof(FVector));

    FString Name = GetClass()->GetName();
    uint32 NameHash = FCrc::StrCrc32(*Name);

    uint32 CombinedHash = HashCombineFast(PosHash, NameHash);

    Seed = static_cast<int32>(CombinedHash);
}

void ABaseDebrisSpawner::OnRep_DebrisArray()
{
    const int32 NewCount = DebrisArray.Instances.Num();

    // 서버 쪽에서 GenerateInstances() 할 때 DebrisArray.Instances.Empty()
    // NewCount < LastReplicatedItemCount 이므로 리셋
    if (NewCount < LastReplicatedItemCount)
    {
        // 클라이언트의 HISM 비우기
        for (auto& Pair : MeshToComponentMap)
        {
            if (Pair.Value)
                Pair.Value->ClearInstances();
        }
        LastReplicatedItemCount = 0;
    }

    // 증분 처리
    for (int32 i = LastReplicatedItemCount; i < NewCount; ++i)
    {
        ApplyDebrisInstance(DebrisArray.Instances[i]);
    }

    // 다음 OnRep 개수 갱신
    LastReplicatedItemCount = NewCount;
}

void ABaseDebrisSpawner::ApplyDebrisInstance(const FDebrisInstanceDataItem& Item)
{
    // 메시 선택
    UStaticMesh* Mesh = nullptr;
    switch (Item.MeshType)
    {
    case EDebrisMeshType::Variant:
        if (MeshVariants.IsValidIndex(Item.MeshIndex))
            Mesh = MeshVariants[Item.MeshIndex];
        break;
    case EDebrisMeshType::Vehicle:
        if (VehicleMeshes.IsValidIndex(Item.MeshIndex))
            Mesh = VehicleMeshes[Item.MeshIndex];
        break;
    case EDebrisMeshType::Other:
        if (OtherMeshes.IsValidIndex(Item.MeshIndex))
            Mesh = OtherMeshes[Item.MeshIndex];
        break;
    }
    if (!Mesh) return;

    // HISM 컴포넌트 확보
    UHierarchicalInstancedStaticMeshComponent* Comp =
        GetOrCreateInstancedMeshComponent(Mesh);
    if (!Comp) return;

    // 월드 -> 로컬 변환 후 인스턴스 추가
    FVector LocalPos = Comp->GetComponentTransform()
        .InverseTransformPosition(Item.Location);
    FTransform TM(Item.Rotation, LocalPos, Item.Scale);
    Comp->AddInstance(TM);
}

