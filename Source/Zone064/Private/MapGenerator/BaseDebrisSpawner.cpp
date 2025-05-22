#include "MapGenerator/BaseDebrisSpawner.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionShape.h"

ABaseDebrisSpawner::ABaseDebrisSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    // BoxComponent (스폰 영역)
    SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    RootComponent = SpawnArea;
    SpawnArea->SetBoxExtent(FVector(250.f, 250.f, 50.f));
    SpawnArea->SetHiddenInGame(true);
    SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create the Instanced Static Mesh Component
    InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
    InstancedMesh->SetupAttachment(SpawnArea);
    InstancedMesh->SetRelativeLocation(FVector::ZeroVector);

    // 기본 충돌 설정
    InstancedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InstancedMesh->SetCollisionResponseToAllChannels(ECR_Block);
    InstancedMesh->SetNotifyRigidBodyCollision(true); // 이벤트 트리거용

    NumInstances = 4;
    SpawnChance = 1.0f;
    VehicleSpawnRatio = 0.1f;
    VehicleSpawnChance = 0.2f;
    bUseSplitMeshArrays = false;
}

void ABaseDebrisSpawner::BeginPlay()
{
    Super::BeginPlay();

    GenerateInstances();
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
    FVector Origin = SpawnArea->GetComponentLocation();
    FVector Extent = SpawnArea->GetScaledBoxExtent();

    // 인스턴스 위치/반경 저장 배열
    TArray<FVector> PlacedLocations;
    TArray<float> PlacedRadii;

    // case 1 통합 랜덤 스폰 : MeshVariants
    if (!bUseSplitMeshArrays)
    {
        if (MeshVariants.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("MeshVariants 배열이 비어 있습니다."));
            return;
        }

        int32 SpawnedCount = 0;
        int32 MaxAttempts = NumInstances * 5;



        for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
        {
            if (FMath::FRand() > SpawnChance)
                continue;

            FVector LocalRandom = FVector(
                FMath::FRandRange(-Extent.X, Extent.X),
                FMath::FRandRange(-Extent.Y, Extent.Y),
                0.f
            );

            FVector Start = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, 500.f));
            FVector End = SpawnTransform.TransformPosition(LocalRandom + FVector(0, 0, -1000.f));

            FHitResult HitResult;
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(this);

            if (!World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams))
                continue;

            UStaticMesh* ChosenMesh = MeshVariants[FMath::RandRange(0, MeshVariants.Num() - 1)];
            if (!ChosenMesh) continue;

            float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();

            FVector SpawnLocation = HitResult.ImpactPoint;
            FRotator RandomRot(0, FMath::FRandRange(0.f, 360.f), 0);

            // 자기 스포너 내에서 충돌 검사
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
            if (bBlocked) continue;

            UInstancedStaticMeshComponent* MeshComp = GetOrCreateInstancedMeshComponent(ChosenMesh);
            if (!MeshComp) continue;

            FVector LocalPosition = MeshComp->GetComponentTransform().InverseTransformPosition(SpawnLocation);
            FTransform InstanceTransform(RandomRot, LocalPosition);

            ///// 디버그
            FVector BoxExtent = ChosenMesh->GetBounds().BoxExtent;
            //FVector DebugLocation = HitResult.ImpactPoint;

            DrawDebugBox(
                GetWorld(),
                SpawnLocation,
                BoxExtent,
                FColor::Red,
                false,          // bPersistentLines: false → 시간 지나면 사라짐
                30.0f,           // LifeTime
                0,
                2.0f            // 선 두께
            );

            MeshComp->AddInstance(InstanceTransform);

            PlacedLocations.Add(SpawnLocation);
            PlacedRadii.Add(InstanceRadius);

            SpawnedCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("[Simple] %d개의 인스턴스를 생성했습니다."), SpawnedCount);
        return;
    }

    // case 2 분리 랜덤 스폰 
    if (VehicleMeshes.Num() == 0 && OtherMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("VehicleMeshes 또는 OtherMeshes 배열이 비어 있습니다!"));
        return;
    }

    int32 VehicleLimit = FMath::RoundToInt(NumInstances * VehicleSpawnRatio);
    if (FMath::FRand() > VehicleSpawnChance) VehicleLimit = 0.0f;

    int32 VehicleCount = 0;
    int32 SpawnedCount = 0;
    int32 MaxAttempts = NumInstances * 5;

    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < NumInstances; ++Attempt)
    {
        if (FMath::FRand() > SpawnChance)
            continue;

        FVector LocalRandom = FVector(
            FMath::FRandRange(-Extent.X, Extent.X),
            FMath::FRandRange(-Extent.Y, Extent.Y),
            0.f
        );

        FVector Start = Origin + LocalRandom + FVector(0, 0, 500.f);
        FVector End = Origin + LocalRandom + FVector(0, 0, -1000.f);

        FHitResult HitResult;
        FCollisionQueryParams TraceParams;
        TraceParams.AddIgnoredActor(this);

        bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams);
        if (!bHit) continue;

        FVector SpawnLocation = HitResult.ImpactPoint;
        FRotator RandomRot(0, FMath::FRandRange(0.f, 360.f), 0);
        FTransform InstanceTransform(RandomRot, SpawnLocation);

        UStaticMesh* ChosenMesh = nullptr;
        bool bIsVehicle = false;

        if (VehicleCount < VehicleLimit && VehicleMeshes.Num() > 0 && FMath::FRand() < VehicleSpawnRatio)
        {
            int32 Index = FMath::RandRange(0, VehicleMeshes.Num() - 1);
            ChosenMesh = VehicleMeshes[Index];
            bIsVehicle = true;
        }
        else if (OtherMeshes.Num() > 0)
        {
            int32 Index = FMath::RandRange(0, OtherMeshes.Num() - 1);
            ChosenMesh = OtherMeshes[Index];
        }

        if (!ChosenMesh) continue;

        float InstanceRadius = ChosenMesh->GetBounds().BoxExtent.GetMax();

        // 자기 스포너 내에서 충돌 검사
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
        if (bBlocked) continue;

        UInstancedStaticMeshComponent* MeshComp = GetOrCreateInstancedMeshComponent(ChosenMesh);
        if (!MeshComp) continue;

        MeshComp->AddInstance(InstanceTransform);

        PlacedLocations.Add(SpawnLocation);
        PlacedRadii.Add(InstanceRadius);

        if (bIsVehicle) VehicleCount++;
        SpawnedCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("[Split] %d개의 인스턴스를 생성했습니다. (차량 %d개 / 제한 %d개)"), SpawnedCount, VehicleCount, VehicleLimit);
}





UInstancedStaticMeshComponent* ABaseDebrisSpawner::GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh)
{
    if (!Mesh) return nullptr;

    if (MeshToComponentMap.Contains(Mesh))
    {
        return MeshToComponentMap[Mesh];
    }

    FString ComponentName = FString::Printf(TEXT("InstancedMesh_%s"), *Mesh->GetName());
    UInstancedStaticMeshComponent* NewComp = NewObject<UInstancedStaticMeshComponent>(this, *ComponentName);
    NewComp->RegisterComponent();
    NewComp->SetStaticMesh(Mesh);
    NewComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    NewComp->SetCollisionResponseToAllChannels(ECR_Block);
    NewComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    MeshToComponentMap.Add(Mesh, NewComp);
    return NewComp;
}

