#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintDataDefinitions.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectIterator.h"
#include "ZNVoipLibrary.generated.h"

UCLASS()
class ZONE064_API UZNVoipLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "VoipLibrary")
	static void ClearVoicePackets(UObject* WorldContextObject);
};
