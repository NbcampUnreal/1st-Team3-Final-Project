#include "MapGenerator/BaseDebrisSpawner.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "MapGenerator/MapGenerator.h"

ABaseDebrisSpawner::ABaseDebrisSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

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

}

void ABaseDebrisSpawner::BeginPlay()
{
    Super::BeginPlay();

    RandomStream.Initialize(Seed);

    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMapGenerator::StaticClass());
    if (FoundActor)
    {
        AMapGenerator* MapGenerator = Cast<AMapGenerator>(FoundActor);
        if (MapGenerator)
        {
            MapGenerator->OnPropSpawnComplete.AddLambda([this]()
                {
                    GenerateInstances();
                });
        }
    }
    
    //GenerateInstances();
}

void ABaseDebrisSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseDebrisSpawner::GenerateInstances()
{
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
    int32 MaxAttempts = NumInstances * 2;

    // 통합 랜덤 방식
    if (!bUseSplitMeshArrays)
    {
        if (MeshVariants.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("MeshVariants 배열이 비어 있습니다."));
            return;
        }

        for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
        {
            float Roll = RandomStream.FRand();
            if (Roll > SpawnChance)
                continue;

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
                continue;

            int32 Index = RandomStream.RandRange(0, MeshVariants.Num() - 1);
            UStaticMesh* ChosenMesh = MeshVariants.IsValidIndex(Index) ? MeshVariants[Index] : nullptr;
            if (!ChosenMesh)
            {
                RandomStream.FRand();
                continue;
            }

            float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();
            FVector SpawnLocation = HitResult.ImpactPoint;
            FRotator RandomRot(0, RandomStream.FRandRange(0.f, 360.f), 0);

            // 중복 충돌 검사
            bool bBlocked = false;
            for (int32 i = 0; i < PlacedLocations.Num(); ++i)
            {
                float MinDist = InstanceRadius + PlacedRadii[i];
                if (FVector::DistSquared(PlacedLocations[i], SpawnLocation) < FMath::Square(MinDist))
                {
                    bBlocked = true;
                    break;
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
            FTransform InstanceTransform(RandomRot, LocalPos);
            
            // 추가전 SpawnLocation 로그

            MeshComp->AddInstance(InstanceTransform);
            //DrawDebugBox(World, SpawnLocation, ChosenMesh->GetBounds().BoxExtent, FColor::Red, false, 30.0f, 0, 2.0f);

            PlacedLocations.Add(SpawnLocation);
            PlacedRadii.Add(InstanceRadius);
            SpawnedCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("[Simple] %d개의 인스턴스를 생성했습니다."), SpawnedCount);
        return;
    }

    // 분리 방식 (차량 vs 기타)
    if (VehicleMeshes.Num() == 0 && OtherMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VehicleMeshes 또는 OtherMeshes 배열이 비어 있습니다."));
        return;
    }

    int32 VehicleLimit = FMath::RoundToInt(NumInstances * VehicleSpawnRatio);
    float VehicleRoll = RandomStream.FRand();
    if (VehicleRoll > VehicleSpawnChance) VehicleLimit = 0;

    int32 VehicleCount = 0;
    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
    {
        float SpawnChanceRoll = RandomStream.FRand();
        if (SpawnChanceRoll > SpawnChance)
            continue;

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
        FRotator RandomRot(0, RandomStream.FRandRange(0.f, 360.f), 0);

        UStaticMesh* ChosenMesh = nullptr;
        bool bIsVehicle = false;

        float VehicleSpawnRoll = RandomStream.FRand();
        if (VehicleCount < VehicleLimit && VehicleMeshes.Num() > 0 && VehicleSpawnRoll < VehicleSpawnRatio)
        {
            ChosenMesh = VehicleMeshes[RandomStream.RandRange(0, VehicleMeshes.Num() - 1)];
            bIsVehicle = true;
        }
        else if (OtherMeshes.Num() > 0)
        {
            ChosenMesh = OtherMeshes[RandomStream.RandRange(0, OtherMeshes.Num() - 1)];
        }

        if (!ChosenMesh) continue;

        float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();

        // 중복 충돌 검사
        bool bBlocked = false;
        for (int32 i = 0; i < PlacedLocations.Num(); ++i)
        {
            float MinDist = (InstanceRadius + PlacedRadii[i]) * 0.75f;
            if (FVector::DistSquared(PlacedLocations[i], SpawnLocation) < FMath::Square(MinDist))
            {
                bBlocked = true;
                break;
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
        FTransform InstanceTransform(RandomRot, LocalPos);

        MeshComp->AddInstance(InstanceTransform);
        //DrawDebugBox(World, SpawnLocation, ChosenMesh->GetBounds().BoxExtent, FColor::Blue, false, 30.0f, 0, 2.0f);

        PlacedLocations.Add(SpawnLocation);
        PlacedRadii.Add(InstanceRadius);
        if (bIsVehicle) VehicleCount++;
        SpawnedCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("[Split] %d개의 인스턴스를 생성했습니다. (차량 %d개 / 제한 %d개)"), SpawnedCount, VehicleCount, VehicleLimit);
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
    NewComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    NewComp->SetCollisionResponseToAllChannels(ECR_Block);

    MeshToComponentMap.Add(Mesh, NewComp);
    return NewComp;
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
