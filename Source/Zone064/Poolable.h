#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPoolable : public UInterface
{
    GENERATED_BODY()
};

class ZONE064_API IPoolable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poolable")
    void OnPoolBegin();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poolable")
    void OnPoolEnd();
};
