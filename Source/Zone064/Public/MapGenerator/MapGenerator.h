#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "MapGenerator.generated.h"

UCLASS()
class ZONE064_API AMapGenerator : public AActor
{
    GENERATED_BODY()

public:
    AMapGenerator();

protected:
    virtual void BeginPlay() override;

public:
    // �� ũ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridWidth = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridHeight = 5;

    // �� �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TileSize = 1000.0f;

    // Soft reference ����Ʈ
    UPROPERTY(EditAnywhere, Category = "Generation")
    TArray<TSoftClassPtr<AActor>> BlockPrefabAssets;

    // ��Ÿ�� ĳ�̵� ���� Ŭ����
    TArray<TSubclassOf<AActor>> CachedPrefabs;

    // �񵿱� �ε� �� �ݹ�
    void OnPrefabsLoaded();

    // ���� �õ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Seed = 42;

private:
    FRandomStream RandomStream;
    FStreamableManager AssetLoader;

    void GenerateMap();
};
