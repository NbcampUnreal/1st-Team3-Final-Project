#include "SoundOverlapComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

USoundOverlapComponent::USoundOverlapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); 
}

void USoundOverlapComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    OverlapSphere = NewObject<USphereComponent>(Owner, TEXT("OverlapSphere"));
    if (OverlapSphere)
    {
        OverlapSphere->RegisterComponent();
        OverlapSphere->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        OverlapSphere->SetSphereRadius(SphereRadius);
        OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        OverlapSphere->SetCollisionObjectType(ECC_WorldDynamic);
        OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &USoundOverlapComponent::OnOverlapBegin);
        OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &USoundOverlapComponent::OnOverlapEnd);
    }

    AudioComponent = NewObject<UAudioComponent>(Owner, TEXT("AudioComponent"));
    if (AudioComponent)
    {
        AudioComponent->RegisterComponent();
        AudioComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        AudioComponent->bAutoActivate = false;

        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
        }
    }
}

void USoundOverlapComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{

    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        ClientPlaySound();
    }
}

void USoundOverlapComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        ClientStopSound();
    }
}

void USoundOverlapComponent::ClientPlaySound_Implementation()
{
    if (AudioComponent && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
}

void USoundOverlapComponent::ClientStopSound_Implementation()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        const float FadeOutDuration = 1.5f;
        const float FadeToVolume = 0.0f;
        const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear;

        AudioComponent->FadeOut(FadeOutDuration, FadeToVolume, FadeCurve);
    }
}

