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

    // �ǹ� ũ�� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Width = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Depth = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Floors = 2;

    // �޽� ����
    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* FloorMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* DoorMesh;

    UPROPERTY(EditAnywhere, Category = "Generation")
    UStaticMesh* StairMesh;

    // ���� ��ġ ���� (�ܺο��� ȣ��)
    UFUNCTION(BlueprintCallable)
    void SetGridPosition(FIntPoint InGridPos);

    UPROPERTY(BlueprintReadOnly)
    FIntPoint GridPosition;

protected:
    virtual void BeginPlay() override;

    void GenerateStructure();

    // �� ���� ��� ��ġ ���� (�� ��ȣ �� (X,Y))
    TMap<int32, FIntPoint> StairPositions;
};
