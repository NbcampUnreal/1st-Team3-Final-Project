#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "MapGenerator.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPropSpawnComplete);

UENUM(BlueprintType)
enum class EZoneType : uint8
{
    None                UMETA(DisplayName = "None"),
    Road                UMETA(DisplayName = "Road"), // 차도
    Road_Sidewalk       UMETA(DisplayName = "Road_Sidewalk"),  // 인도
    Road_Sidewalk_Traffic       UMETA(DisplayName = "Road_Sidewalk_Traffic"),  // 신호등있는 인도
    Road_Crosswalk      UMETA(DisplayName = "Road_Crosswalk"), // 횡단보도
    Road_Intersection   UMETA(DisplayName = "Road_Intersection"), //교차로
    HighRise3            UMETA(DisplayName = "High Rise 3x3"),
    HighRise4            UMETA(DisplayName = "High Rise 4x4"),
    HighRise5            UMETA(DisplayName = "High Rise 5x5"),
    LowRise             UMETA(DisplayName = "Low Rise"),
    Alley               UMETA(DisplayName = "Alley"),
    Plant               UMETA(DisplayName = "Plant"),
    Special             UMETA(DisplayName = "Special")
};

UENUM(BlueprintType)
enum class ERoadDirection : uint8
{
    None,
    Horizontal,
    Vertical,
    Crossroad
};

USTRUCT(BlueprintType)
struct FGridCellData
{
    GENERATED_BODY()


    EZoneType ZoneType = EZoneType::None;
    FRotator PreferredRotation = FRotator::ZeroRotator;
    bool bIsCrossroad = false;
    int32 CrossroadSize = 0;
    UPROPERTY(BlueprintReadOnly)
    ERoadDirection RoadDirection = ERoadDirection::None; 
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

USTRUCT(BlueprintType)
struct FBuildingSpawnData
{
    GENERATED_BODY()

    UPROPERTY()
    FIntPoint Origin;
    
    UPROPERTY()
    int32 Width;

    UPROPERTY()
    int32 Height;

    UPROPERTY()
    EZoneType ZoneType;

    UPROPERTY()
    FRotator Rotation;
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
    // 스폰 완료시 호출할 델리게이트
    FOnPropSpawnComplete OnPropSpawnComplete;

    // 맵 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 GridHeight;

    // 블럭 간 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")

    float TileSize;

    // 에디터에서 사용할 소프트 레퍼 리스트
    UPROPERTY(EditAnywhere, Category = "Generation")
    TArray<FZonePrefabSet> BlockPrefabSets;
 
    TMap<EZoneType, TArray<TSoftClassPtr<AActor>>> BlockPrefabAssetsByZone;
    TMap<EZoneType, TArray<TSubclassOf<AActor>>> CachedPrefabsByZone;

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> PropClasses;

    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> TreePrefabs;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> LightPrefabs;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> TrafficPrefabs;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> TrashPrefabs;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<AActor> FencePrefab;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TSubclassOf<AActor> BuildingGroundPrefab;
    UPROPERTY(EditAnywhere, Category = "Generation|Props")
    TArray<TSubclassOf<AActor>> InfraPrefabs;

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

    bool IsAreaAvailable(FIntPoint TopLeft, int32 Width, int32 Height, const TArray<EZoneType>& BlockedTypes);
    
    void MarkZone(FIntPoint TopLeft, int32 Width, int32 Height, EZoneType ZoneType, FRotator Rotation);

    UFUNCTION(BlueprintCallable)
    void DrawDebugZoneMap();

    void SpawnSidewalkProps();

    void TrySpawnProps(AActor* Target, FIntPoint GridPos);
    void TrySpawnBorder(const FVector& Location, const FIntPoint& GridPos);
    
    // 회전값 고려한 좌상단 구하기
    FIntPoint GetTopLeftFromOrigin(FIntPoint center, int32 width, int32 height, const FRotator& Rotation);
    FVector GetWorldCenterFromTopLeft(FIntPoint TopLeft, int32 Width, int32 Height, FRotator Rotation);
    FVector GetWorldFromGrid(FIntPoint GridPos);

    // 커스텀 플레이어 스타트 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<APlayerStart> PlayerStartActor;
    
    // 구역 맵
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<FIntPoint, FGridCellData> ZoneMap;

private:
    FRandomStream RandomStream;
    FStreamableManager AssetLoader;
    
    // 4방향 탐색용 오프셋
    TArray<FIntPoint> SearchOffsetList;
    // 대각선 방향 탐색용 오프셋
    TArray<FIntPoint> CornerOffsetList;
    TMap<FIntPoint, float> CornerYawMap;

    // 데브리 스폰용 Road Array
    TArray<FIntPoint> FinalRoadArray;
    // Road Debris InstancedStaticMeshComponent

    // 건물 스폰 리스트
    TArray<FBuildingSpawnData> BuildingSpawnList;

    void GenerateMap();
    void GenerateZoneMap();
    void AddtoBuildingSpawnList(FIntPoint Pos, int32 Width, int32 Height, EZoneType ZoneType, FRotator Rotation);

    // 교차로 최소 간격
    int32 CrossroadMinSpacing;
    // 후생성 횡단보도 확률
    float CrosswalkChance;

    float TreeSpawnChance;
    float LightSpawnChance;
    float TrashSpawnChance;
    float TrafficSpawnChance;
    float InfraSpawnChance;

    // 프랍 최소 간격
    int32 LightSpawnSpacing;
    int32 TreeSpawnSpacing;
    int32 InfraSpawnSpacing;


};
