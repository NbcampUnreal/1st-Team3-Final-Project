#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoundOverlapComponent.generated.h"

class USphereComponent;
class UAudioComponent;
class USoundCue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZONE064_API USoundOverlapComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USoundOverlapComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    USphereComponent* OverlapSphere;

    UPROPERTY()
    UAudioComponent* AudioComponent;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 멀티캐스트
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlaySound();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStopSound();

    int32 OverlappingCharacters = 0;

    static TMap<USoundCue*, int32> SoundCueActiveCount;

public:
    UPROPERTY(EditAnywhere, Category = "Sound")
    float SphereRadius = 300.f;

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, Category = "Sound")
    int32 MaxInstancesPerCue = 3;
};
