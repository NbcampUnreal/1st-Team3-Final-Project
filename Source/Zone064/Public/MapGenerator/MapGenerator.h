#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "MapGenerator.generated.h"


UENUM(BlueprintType)
enum class EZoneType : uint8
{
    None        UMETA(DisplayName = "None"),
    Road        UMETA(DisplayName = "Road"),
    HighRise    UMETA(DisplayName = "High Rise"),
    LowRise     UMETA(DisplayName = "Low Rise"),
    Special     UMETA(DisplayName = "Special")
};

USTRUCT(BlueprintType)
struct FGridCellData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EZoneType ZoneType = EZoneType::None;

    UPROPERTY(BlueprintReadWrite)
    bool bIsCrossroad = false;

    UPROPERTY(BlueprintReadWrite)
    bool bUsed = false;

    UPROPERTY(BlueprintReadWrite)
    FRotator PreferredRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FZonePrefabSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EZoneType ZoneType;

    UPROPERTY(EditAnywhere)
    TArray<TSoftClassPtr<AActor>> Prefabs;
};

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
    float TileSize = 500.0f;

    // 에디터에서 사용할 소프트 레퍼 리스트
    UPROPERTY(EditAnywhere, Category = "Generation")
    TArray<FZonePrefabSet> BlockPrefabSets;
 
    TMap<EZoneType, TArray<TSoftClassPtr<AActor>>> BlockPrefabAssetsByZone;
    TMap<EZoneType, TArray<TSubclassOf<AActor>>> CachedPrefabsByZone;

    // 비동기 로딩 후 콜백
    void OnPrefabsLoaded();

    // 랜덤 시드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Seed;

    void AssignSpecialClusters();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation|Special")
    int32 RequiredClusterSize;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation|Special")
    float SpecialChance;


private:
    FRandomStream RandomStream;
    FStreamableManager AssetLoader;
    // 구역 맵
    TMap<FIntPoint, FGridCellData> ZoneMap;

    void GenerateMap();
    void GenerateZoneMap();
};
