// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibrary/ZNAIPerceptionLibrary.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"


void UZNAIPerceptionLibrary::SetSightRadius(UAIPerceptionComponent* PerceptionComp, float NewSightRadius, float LoseSightOffset)
{
    if (!PerceptionComp) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Perception Comp not found"));
        return;
    }

    if (UAISenseConfig_Sight* SightConfig = PerceptionComp->GetSenseConfig<UAISenseConfig_Sight>())
    {
        SightConfig->SightRadius = NewSightRadius;
        SightConfig->LoseSightRadius = NewSightRadius + LoseSightOffset;
        PerceptionComp->RequestStimuliListenerUpdate();
        UE_LOG(LogTemp, Display, TEXT("[AISenseConfig_Sight] Radius Changed : %f"), SightConfig->SightRadius);
    }
}

void UZNAIPerceptionLibrary::SetHearingRadius(UAIPerceptionComponent* PerceptionComp, float NewHearingRange)
{
    if (!PerceptionComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perception Comp not found"));
        return;
    }
    if (UAISenseConfig_Hearing* HearingConfig = PerceptionComp->GetSenseConfig<UAISenseConfig_Hearing>())
    {
        HearingConfig->HearingRange = NewHearingRange;
        PerceptionComp->RequestStimuliListenerUpdate();
        UE_LOG(LogTemp, Display, TEXT("[AISenseConfig_Hearing] Range Changed : %f"), HearingConfig->HearingRange);
    }
}
