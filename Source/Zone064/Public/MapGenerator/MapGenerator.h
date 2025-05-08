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
    // 맵 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridWidth = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridHeight = 5;

    // 블럭 간 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float TileSize = 1000.0f;

    // Soft reference 리스트
    UPROPERTY(EditAnywhere, Category = "Generation")
    TArray<TSoftClassPtr<AActor>> BlockPrefabAssets;

    // 런타임 캐싱된 실제 클래스
    TArray<TSubclassOf<AActor>> CachedPrefabs;

    // 비동기 로딩 후 콜백
    void OnPrefabsLoaded();

    // 랜덤 시드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Seed = 42;

private:
    FRandomStream RandomStream;
    FStreamableManager AssetLoader;

    void GenerateMap();
};
