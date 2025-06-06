#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseDebrisSpawner.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UBoxComponent;
class UStaticMesh;
class AMapGenerator;

UENUM()
enum class EDebrisMeshType : uint8
{
    Variant,
    Vehicle,
    Other
};

USTRUCT()
struct FDebrisInstanceData 
{

    GENERATED_BODY()

    UPROPERTY() FVector Location;
    UPROPERTY() FRotator Rotation;
    UPROPERTY() int32 MeshIndex;
    UPROPERTY() EDebrisMeshType MeshType;
};

UCLASS()
class ZONE064_API ABaseDebrisSpawner : public AActor
{
    GENERATED_BODY()

public:
    ABaseDebrisSpawner();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Instancing")
    void GenerateInstances();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    
    AMapGenerator* MapGenerator;

    UFUNCTION()
    bool CheckCrossRoad();

    virtual void OnConstruction(const FTransform& Transform) override;
    
    UPROPERTY(ReplicatedUsing = OnRep_DebrisSpawnData)
    TArray<FDebrisInstanceData> ReplicatedInstances;

    // 클라이언트 OnRep
    UFUNCTION()
    void OnRep_DebrisSpawnData();

    // 메시별 인스턴싱 컴포넌트 매핑
    UPROPERTY()
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> MeshToComponentMap;

    UHierarchicalInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);

    // 랜덤 스트림과 시드 (멀티에서 동일한 환경으로 스폰하려면 같은 시드여야함)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FRandomStream RandomStream;

    // 스폰 영역
    UPROPERTY(EditAnywhere, Category = "Instancing")
    UBoxComponent* SpawnArea;

    // 메시 배열들
    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> MeshVariants;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> VehicleMeshes;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> OtherMeshes;

    // 설정 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instancing")
    int32 NumInstances;

    UPROPERTY(EditAnywhere, Category = "Instancing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnChance;

    UPROPERTY(EditAnywhere, Category = "Instancing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VehicleSpawnRatio;

    UPROPERTY(EditAnywhere, Category = "Instancing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VehicleSpawnChance;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    bool bUseSplitMeshArrays;

    
};
