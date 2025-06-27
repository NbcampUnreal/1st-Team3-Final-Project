#include "SoundOverlapComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

TMap<USoundCue*, int32> USoundOverlapComponent::SoundCueActiveCount;

USoundOverlapComponent::USoundOverlapComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); // 네트워크 복제
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
    if (!GetOwner()->HasAuthority()) return;

    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        OverlappingCharacters++;

        if (OverlappingCharacters == 1 && SoundCue)
        {
            int32& CurrentCount = SoundCueActiveCount.FindOrAdd(SoundCue);
            if (CurrentCount < MaxInstancesPerCue)
            {
                MulticastPlaySound();
                CurrentCount++;
            }
        }
    }
}

void USoundOverlapComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!GetOwner()->HasAuthority()) return;

    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        OverlappingCharacters = FMath::Max(0, OverlappingCharacters - 1);

        if (OverlappingCharacters == 0 && AudioComponent && AudioComponent->IsPlaying())
        {
            MulticastStopSound();

            if (SoundCueActiveCount.Contains(SoundCue))
            {
                int32& Count = SoundCueActiveCount[SoundCue];
                Count = FMath::Max(0, Count - 1);
            }
        }
    }
}

void USoundOverlapComponent::MulticastPlaySound_Implementation()
{
    if (AudioComponent && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
}

void USoundOverlapComponent::MulticastStopSound_Implementation()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        const float FadeOutDuration = 1.5f;
        const float FadeToVolume = 0.0f;
        const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear;

        AudioComponent->FadeOut(FadeOutDuration, FadeToVolume, FadeCurve);
    }
}

