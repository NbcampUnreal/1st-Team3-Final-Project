#include "MapGenerator/EnvironmentManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

AEnvironmentManager::AEnvironmentManager()
{
    bReplicates = true;

    Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FogBox"));
    SetRootComponent(Box);
    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Box->SetMaterial(0, nullptr);

    bShouldRain = false;
}

void AEnvironmentManager::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World) UE_LOG(LogTemp, Warning, TEXT("월드 없음"));

    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ADirectionalLight::StaticClass());
    if (FoundActor)
    {
        TargetDirectionalLight = Cast<ADirectionalLight>(FoundActor);
        UE_LOG(LogTemp, Display, TEXT("DirectionalLight 서치 완료"));
    }
    else if (!FoundActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectionalLight missing"));
    }
    
}

void AEnvironmentManager::SetLightRotation(FRotator NewRotation)
{
    // 서버에서만 실행되어야 함
    if (HasAuthority())
    {
        LightRotation = NewRotation;
        OnRep_LightRotation(); // 서버에서 직접 적용
    }
}

void AEnvironmentManager::SetLightIntensity(float NewIntensity)
{
    {
        LightIntensity = NewIntensity;
        OnRep_LightIntensity(); // 서버에서 직접 적용
    }
}

void AEnvironmentManager::SetLightColor(FLinearColor NewLightColor)
{
    LightColor = NewLightColor;
    OnRep_LightColor();
}

void AEnvironmentManager::SetMaterialInddex(int32 Index)
{
    MaterialIndex = Index;
    OnRep_MaterialIndex();
}

void AEnvironmentManager::SetShouldRain(bool NewShouldRain)
{
    bShouldRain = NewShouldRain;
    OnRep_bShouldRain();
}

void AEnvironmentManager::OnRep_LightRotation()
{
    if (TargetDirectionalLight)
    {
        TargetDirectionalLight->SetActorRotation(LightRotation);
    }
}

void AEnvironmentManager::OnRep_LightIntensity()
{
    if (TargetDirectionalLight)
    {
        TargetDirectionalLight->GetLightComponent()->SetIntensity(LightIntensity);
    }
}

void AEnvironmentManager::OnRep_LightColor()
{
    if (TargetDirectionalLight)
    {
        TargetDirectionalLight->GetLightComponent()->SetLightColor(LightColor);
    }
}

void AEnvironmentManager::OnRep_MaterialIndex()
{
    if (Box && MaterialArray.IsValidIndex(MaterialIndex))
    {
        Box->SetMaterial(0, MaterialArray[MaterialIndex]);
    }
}
////  캐릭터 클래스가 BP이므로 블루프린트에서 구현
//void AEnvironmentManager::OnRep_bShouldRain()
//{
//}



void AEnvironmentManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AEnvironmentManager, LightRotation);
    DOREPLIFETIME(AEnvironmentManager, LightIntensity);
    DOREPLIFETIME(AEnvironmentManager, LightColor);
    DOREPLIFETIME(AEnvironmentManager, MaterialIndex);
    DOREPLIFETIME(AEnvironmentManager, bShouldRain);
}
