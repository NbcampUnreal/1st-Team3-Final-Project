#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "MapGenerator/MapGenerator.h"
#include "GlobalDebrisGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class AMapGenerator;

USTRUCT()
struct FGlobalDebrisSpawnData
{
    GENERATED_BODY()

    UPROPERTY() FVector   Location;
    UPROPERTY() uint16    RotationYaw;
    UPROPERTY() FVector   Scale = FVector(1.f, 1.f, 1.f);
    UPROPERTY() int32     MeshIndex = INDEX_NONE;
};

UCLASS()
class ZONE064_API AGlobalDebrisGenerator : public AActor
{
    GENERATED_BODY()

public:
    AGlobalDebrisGenerator();

    UFUNCTION(BlueprintCallable)
    void StartGenerateDebris();
    
    void GatherTiles();

    void CreateHISMComponents();

    void SpawnAllDebris();

    virtual void BeginPlay() override;

    UPROPERTY()
    AMapGenerator* MapGenerator = nullptr;

    // 타일 정보
    struct FTileInfo { FVector Center, Extent; };
    TArray<FTileInfo> TileInfos;

    // 어떤 ZoneType 타일에만 뿌릴지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris")
    EZoneType TargetZoneType;

    // 메시 리스트
    UPROPERTY(EditAnywhere, Category = "Debris")
    TArray<UStaticMesh*> MeshVariants;

    // 타일당 스폰 메시 수
    UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "1"))
    int32 NumPerTile = 10;

    // 개당 스폰 확률
    UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "0"))
    float SpawnChance = 1.0f;

    // 메시 랜덤 선택 수
    UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "1"))
    int32 Iterations = 10;

    // 스폰시 충돌 검사 여부
    UPROPERTY(EditAnywhere, Category = "Debris")
    bool bShouldCheckCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris")
    int32 Seed = 12345;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debris")
    FRandomStream RandomStream;

    TArray<UHierarchicalInstancedStaticMeshComponent*> HISMComponents;


    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SpawnDebrisBatch(const TArray<FGlobalDebrisSpawnData>& InBatch);
    void Multicast_SpawnDebrisBatch_Implementation(const TArray<FGlobalDebrisSpawnData>& InBatch);

    void SpawnLocalInstance(const FGlobalDebrisSpawnData& D);

    UPROPERTY(EditAnywhere, Category = "Debris")
    int32 RPCChunkSize = 25;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_FinalizeSpawn();
    void Multicast_FinalizeSpawn_Implementation();

    // 대용량 데이터 나눠서 뿌리기
    TArray<FGlobalDebrisSpawnData> Batch;
    FTimerHandle BatchTimerHandle;
    int32 PendingBatchStart;

    void SendNextChunk();

};
