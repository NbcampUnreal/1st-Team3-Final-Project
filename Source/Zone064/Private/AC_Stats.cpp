#include "AC_Stats.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UAC_Stats::UAC_Stats()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 이 컴포넌트가 네트워크에서 복제되도록 설정
    SetIsReplicatedByDefault(true);
}

// 시작 시 실행 (델리게이트 바인딩 및 위젯 초기화)
void UAC_Stats::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 데미지 이벤트에 함수 바인딩 (모든 데미지 수신 시 호출)
    Owner->OnTakeAnyDamage.AddDynamic(this, &UAC_Stats::HandleTakeAnyDamage);

    // 체력바 위젯 초기화 이벤트 호출 (BP에서 처리 가능)
    OnHealthWidgetInit.Broadcast();
}

// 매 프레임마다 실행 (스태미나 자동 회복)
void UAC_Stats::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 스태미나 자동 회복
    if (Stamina < MaxStamina)
    {
        Stamina += StaminaRegenRate * DeltaTime;
        Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
    }
}

// 방어력을 반영한 데미지 계산
float UAC_Stats::CalculateDamage(float IncomingDamage) const
{
    float FinalDamage = IncomingDamage - Defense;
    return FMath::Max(0.0f, FinalDamage);
}

// 체력 감소 처리
void UAC_Stats::ApplyDamage(float DamageAmount)
{
    Health -= DamageAmount;
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("피해: %.1f → 체력: %.1f"), DamageAmount, Health);

    // 체력 UI 위젯 갱신을 위해 델리게이트 호출
    OnDamageTaken.Broadcast();

    // 체력이 0 이하이면 사망 이벤트 호출
    if (Health <= 0.0f)
    {
        OnCharacterDied.Broadcast();
    }
}

// 스태미나 소모 처리
bool UAC_Stats::ConsumeStamina(float Amount)
{
    if (Stamina >= Amount)
    {
        Stamina -= Amount;
        return true;
    }
    return false;
}

// 스태미나 수동 회복
void UAC_Stats::RecoverStamina(float Amount)
{
    Stamina += Amount;
    Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
}

// 공격력 증가/감소 처리 (장비 연동용)
void UAC_Stats::AddAttackPower(float Value)
{
    AttackPower += Value;
}

// 방어력 증가/감소 처리 (장비 연동용)
void UAC_Stats::AddDefense(float Value)
{
    Defense += Value;
}

// 낙하 속도 기반 데미지 계산
float UAC_Stats::CalculateFallDamage()
{
    AActor* Owner = GetOwner();
    if (!Owner) return 0.0f;

    float Speed = Owner->GetVelocity().Size();

    float Ratio = (Speed - FallDamageMinimumVelocity) / (FallDamageMaximumVelocity - FallDamageMinimumVelocity);
    Ratio = FMath::Clamp(Ratio, 0.0f, 1.0f);

    return FMath::Lerp(0.0f, MaxHealth, Ratio);
}

// 데미지를 수신했을 때 자동 호출
void UAC_Stats::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f) return;

    // 데미지 적용
    ApplyDamage(Damage);

    // 컨트롤러 존재 여부 확인
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* CharacterOwner = Cast<ACharacter>(Owner);
    if (!CharacterOwner || CharacterOwner->IsPendingKillPending()) return;

    AController* Controller = CharacterOwner->GetController();
    if (!Controller) return;

    FVector Location = Owner->GetActorLocation();
    FName Tag = NAME_None;

    // 여기에 ReportDamageEvent 또는 AI 인식 이벤트 호출 가능
}

// 클라이언트 측 체력 동기화 처리 (RepNotify)
void UAC_Stats::OnRep_Health()
{
    UE_LOG(LogTemp, Log, TEXT("[클라이언트] 체력 동기화: %.1f"), Health);

    // 체력 UI 갱신을 위한 델리게이트 호출
    OnDamageTaken.Broadcast();
}

// 복제 대상 등록 함수
void UAC_Stats::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Health 변수 복제 등록
    DOREPLIFETIME(UAC_Stats, Health);
}
