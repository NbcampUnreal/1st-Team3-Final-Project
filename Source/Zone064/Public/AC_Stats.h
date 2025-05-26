#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "AC_Stats.generated.h"

// 체력바 위젯 생성용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthWidgetInit);

// 캐릭터 사망 시 호출
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDied);

// 체력 변화 시 블루프린트에 전달
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageTaken);

UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZONE064_API UAC_Stats : public UActorComponent
{
    GENERATED_BODY()

public:
    UAC_Stats();

protected:

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 체력 변경 시 클라이언트에 반영
    UFUNCTION()
    void OnRep_Health();

    // 복제될 속성 등록 함수
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

    // 현재 체력 (네트워크에서 복제)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Health, Category = "Stats")
    float Health = 100.0f;

    // 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackPower = 20.0f;

    // 방어력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Defense = 10.0f;

    // 최대 스태미나
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float MaxStamina = 100.0f;

    // 현재 스태미나
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float Stamina = 100.0f;

    // 초당 자동 회복 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
    float StaminaRegenRate = 10.0f;

    // 낙하 데미지 최소 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallDamage")
    float FallDamageMinimumVelocity = 600.0f;

    // 낙하 데미지 최대 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallDamage")
    float FallDamageMaximumVelocity = 1200.0f;

    // 체력바 위젯 생성 알림
    UPROPERTY(BlueprintAssignable, Category = "UI")
    FOnHealthWidgetInit OnHealthWidgetInit;

    // 캐릭터 사망 알림
    UPROPERTY(BlueprintAssignable, Category = "Death")
    FOnCharacterDied OnCharacterDied;

    // 체력 변경 알림 (예: UI 업데이트)
    UPROPERTY(BlueprintAssignable, Category = "Damage")
    FOnDamageTaken OnDamageTaken;

    // 스태미나 소비 함수
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool ConsumeStamina(float Amount);

    // 스태미나 수동 회복 함수
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void RecoverStamina(float Amount);

    // 방어력 적용 데미지 계산
    UFUNCTION(BlueprintCallable, Category = "Stats")
    float CalculateDamage(float IncomingDamage) const;

    // 체력 감소 처리
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ApplyDamage(float DamageAmount);

    // 공격력 증가/감소 (장비 영향)
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddAttackPower(float Value);

    // 방어력 증가/감소 (장비 영향)
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddDefense(float Value);

    // 낙하 데미지 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Damage")
    float CalculateFallDamage();

    // 데미지 수신 시 처리 함수
    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

};
