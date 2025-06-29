#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "ZNAIPerceptionLibrary.generated.h"


UCLASS()
class ZONE064_API UZNAIPerceptionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	static void SetSightRadius(UAIPerceptionComponent* PerceptionComp, float NewSightRadius, float LoseSightOffset = 200.0f);
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	static void SetHearingRadius(UAIPerceptionComponent* PerceptionComp, float NewHearingRange);
};
