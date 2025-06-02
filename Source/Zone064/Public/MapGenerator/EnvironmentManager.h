#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentManager.generated.h"

UCLASS()
class ZONE064_API AEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentManager();

protected:
    virtual void BeginPlay() override;

public:

    /*
    *   리플리케이트 시 추가해야 할 것
    *   1. RPC로 전달할 변경된 변수
    *   2. 전달받은 변수로 서버에서 세팅할 함수
    *   3. RepNotify
    */

    // --- Replicated 변수 --- //
    // 시간
    UPROPERTY(ReplicatedUsing = OnRep_LightRotation)
    FRotator LightRotation;

    // 밝기
    UPROPERTY(ReplicatedUsing = OnRep_LightIntensity)
    float LightIntensity;

    // 조명색상(안개 환경만)
    UPROPERTY(ReplicatedUsing = OnRep_LightColor)
    FLinearColor LightColor;

    // 환경 머티리얼 인덱스
    UPROPERTY(ReplicatedUsing = OnRep_MaterialIndex)
    int32 MaterialIndex;

    // 비
    UPROPERTY(ReplicatedUsing = OnRep_bShouldRain, BlueprintReadWrite)
    bool bShouldRain;

    // ExponentialFogHeight = 안개 관련
    UPROPERTY(ReplicatedUsing = OnRep_FogMaxOpacity)
    float FogMaxOpacity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class AExponentialHeightFog* FogActor;

    // 라이트 액터 포인터
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class ADirectionalLight* TargetDirectionalLight;

    // 머티리얼 적용할 박스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* Box;

    // 플레이어 캐릭터
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> PlayerCharacterClass;

    // 머티리얼 보관
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UMaterialInterface*> MaterialArray;

    // 서버에서 직접 세팅 함수
    UFUNCTION(BlueprintCallable)
    void SetLightRotation(FRotator NewRotation);
    UFUNCTION(BlueprintCallable)
    void SetLightIntensity(float NewIntensity);
    UFUNCTION(BlueprintCallable)
    void SetLightColor(FLinearColor NewLightColor);
    UFUNCTION(BlueprintCallable)
    void SetMaterialInddex(int32 Index);
    UFUNCTION(BlueprintCallable)
    void SetShouldRain(bool NewShouldRain);
    UFUNCTION(BlueprintCallable)
    void SetFogMaxOpacity(float NewOpacity);

    // RepNotify
    UFUNCTION()
    void OnRep_LightRotation();
    UFUNCTION()
    void OnRep_LightIntensity();
    UFUNCTION()
    void OnRep_LightColor();
    UFUNCTION()
    void OnRep_MaterialIndex();
    UFUNCTION(BlueprintImplementableEvent)
    void OnRep_bShouldRain();
    UFUNCTION()
    void OnRep_FogMaxOpacity();


    // Replication 셋업
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
