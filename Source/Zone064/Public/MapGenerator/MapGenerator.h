#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "MapGenerator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPropSpawnComplete);

USTRUCT(BlueprintType)
struct FSpawnQueueData
{
    GENERATED_BODY()

    UPROPERTY()    TSoftClassPtr<AActor> ActorClass;
    UPROPERTY()    FVector Location;
    UPROPERTY()    FRotator Rotation;
};

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
    Shop3            UMETA(DisplayName = "Shop 3x3"),
    Shop4            UMETA(DisplayName = "Shop 4x4"),
    Shop5            UMETA(DisplayName = "Shop 5x5"),
    House3            UMETA(DisplayName = "House 3x3"),
    House4            UMETA(DisplayName = "House 4x4"),
    House5            UMETA(DisplayName = "House 5x5"),
    LowRise             UMETA(DisplayName = "Low Rise"),
    Alley               UMETA(DisplayName = "Alley"),
    Garbage               UMETA(DisplayName = "Garbage"),
    Plant               UMETA(DisplayName = "Plant"),
    Farmland               UMETA(DisplayName = "Farmland"),
    Greenhouse               UMETA(DisplayName = "Greenhouse"),
    FarmSlope               UMETA(DisplayName = "FarmSlope"),
    FarmSlopeCorner               UMETA(DisplayName = "FarmSlopeCorner"),
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

    UPROPERTY(BlueprintReadOnly)    EZoneType ZoneType = EZoneType::None;
    UPROPERTY(BlueprintReadOnly)    FRotator PreferredRotation = FRotator::ZeroRotator;
    UPROPERTY(BlueprintReadOnly)    bool bIsCrossroad = false;
    UPROPERTY(BlueprintReadOnly)    int32 CrossroadSize = 0;
    UPROPERTY(BlueprintReadOnly)    ERoadDirection RoadDirection = ERoadDirection::None; 
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
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:

    /*
    *  순차 스폰용 
    */
    UPROPERTY(EditAnywhere, Category = "Generation")
    bool bIsUsingSpawnQueue = false;
    TQueue<FSpawnQueueData> SpawnQueue;
    FTimerHandle SpawnQueueHandle;
    UPROPERTY(EditAnywhere, Category = "Generation")
    float SpawnInterval = 0.1f;
    void EnqueueSpawnData(const FSpawnQueueData& Data);
    void ProcessNextSpawn();
    void StartSpawnSequence();


    UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
    void StartGenerateMap(int32 GenerateSeed);
    void StartGenerateMap_Implementation(int32 GenerateSeed);



    /*
    *   1. 게임모드 StartGenerateMap() 호출
    *   2. 서버->클라이언트에게 멀티캐스트로 비동기 로드 명령 : Multicast_RequestPrefabLoad()
    *   3. 서버 자신도 비동기 로드 수행
    *   4. 로딩이 끝나면 OnPrefabsLoaded() 호출
    *   4-1. 서버는 ReadyClient에 자기 자신 추가
    *   4-2. 클라이언트는 Server_ReportPrefabLoaded()를 통해 ReadyClient에 추가
    *   5. ReadyClient와 GS의 PlayerArray가 같아지면 맵 스폰 개시
    */
    UPROPERTY()
    TSet<APlayerState*> ReadyClients;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_RequestPrefabLoad();
    void Multicast_RequestPrefabLoad_Implementation();

    void RequestAsyncLoadAllPrefabs();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ReportPrefabLoaded();
    bool Server_ReportPrefabLoaded_Validate() { return true; }
    void Server_ReportPrefabLoaded_Implementation();

    void TryStartWhenAllReady();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PropSpawnComplete();
    void Multicast_PropSpawnComplete_Implementation();
    

    // 스폰 완료시 호출할 델리게이트
    UPROPERTY(BlueprintAssignable)
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

    // 랜덤 시드와 setter
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 Seed;

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void SetRandomSeed(int32 NewSeed);

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
    virtual void TrySpawnBorder(const FVector& Location, const FIntPoint& GridPos);
    
    // 회전값 고려한 좌상단 구하기
    FIntPoint GetTopLeftFromOrigin(FIntPoint center, int32 width, int32 height, const FRotator& Rotation);
    FVector GetWorldCenterFromTopLeft(FIntPoint TopLeft, int32 Width, int32 Height, FRotator Rotation);
    FVector GetWorldFromGrid(FIntPoint GridPos);

    // 커스텀 플레이어 스타트 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> PlayerStartActor;
   

    // 구역 맵
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TMap<FIntPoint, FGridCellData> ZoneMap;

    FRandomStream RandomStream;
    TSharedPtr<FStreamableHandle> PrefabLoadHandle;
    
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
    virtual void GenerateZoneMap();
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
