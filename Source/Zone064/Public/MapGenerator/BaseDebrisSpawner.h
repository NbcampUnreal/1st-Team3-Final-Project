#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseDebrisSpawner.generated.h"

UCLASS()
class ZONE064_API ABaseDebrisSpawner : public AActor
{
    GENERATED_BODY()

public:
    ABaseDebrisSpawner();

protected:
    virtual void BeginPlay() override;

protected:
    // 메시별 인스턴싱 컴포넌트 매핑
    UPROPERTY()
    TMap<UStaticMesh*, UInstancedStaticMeshComponent*> MeshToComponentMap;

    UInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);

public:
    virtual void Tick(float DeltaTime) override;

    // Instanced Static Mesh Component
    UPROPERTY(VisibleAnywhere, Category = "Instancing")
    class UInstancedStaticMeshComponent* InstancedMesh;

    // 스폰 영역
    UPROPERTY(EditAnywhere, Category = "Instancing")
    class UBoxComponent* SpawnArea;

    // 메시 저장 배열
    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> MeshVariants;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> VehicleMeshes;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    TArray<UStaticMesh*> OtherMeshes;

    UPROPERTY(EditAnywhere, Category = "Instancing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VehicleSpawnRatio;
    
    // 차량 스폰 확률
    UPROPERTY(EditAnywhere, Category = "Instancing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VehicleSpawnChance;

    UPROPERTY(EditAnywhere, Category = "Instancing")
    bool bUseSplitMeshArrays;

    // 스폰 인스턴스 개수
    UPROPERTY(EditAnywhere, Category = "Instancing")
    int32 NumInstances;

    // 스폰 확률
    UPROPERTY(EditAnywhere, Category = "Instancing")
    float SpawnChance;
    
    UFUNCTION(BlueprintCallable, Category = "Instancing")
    void GenerateInstances();
};
