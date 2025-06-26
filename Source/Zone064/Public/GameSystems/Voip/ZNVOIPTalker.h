#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "ZNVOIPTalker.generated.h"

UCLASS()
class ZONE064_API UZNVOIPTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
	virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VOIP")
	float VolumeMultiplier = 10.0f;
};
