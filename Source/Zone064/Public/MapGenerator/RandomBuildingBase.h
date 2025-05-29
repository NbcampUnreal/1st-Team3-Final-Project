#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomBuildingBase.generated.h"

UCLASS()
class ZONE064_API ARandomBuildingBase : public AActor
{
    GENERATED_BODY()

public:
    ARandomBuildingBase();

    // 건물 크기 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Width = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Depth = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Floors = 2;

    // 메시 에셋
    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* FloorMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* DoorMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* StairMesh;

    // 격자 위치 설정 (외부에서 호출)
    UFUNCTION(BlueprintCallable)
    void SetGridPosition(FIntPoint InGridPos);

    UPROPERTY(BlueprintReadOnly)
    FIntPoint GridPosition;

protected:
    virtual void BeginPlay() override;

    void GenerateStructure();

    // 각 층의 계단 위치 저장 (층 번호 → (X,Y))
    TMap<int32, FIntPoint> StairPositions;
};
