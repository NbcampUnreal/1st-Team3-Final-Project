#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseDebrisSpawner.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UBoxComponent;
class UStaticMesh;

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

protected:
    // 메시별 인스턴싱 컴포넌트 매핑
    UPROPERTY()
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> MeshToComponentMap;

    UHierarchicalInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);

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
    UPROPERTY(EditAnywhere, Category = "Instancing")
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
