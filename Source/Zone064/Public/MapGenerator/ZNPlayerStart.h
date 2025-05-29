#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "ZNPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API AZNPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:

	AZNPlayerStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* CarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USceneComponent*> SpawnPoints;

	UFUNCTION(BlueprintCallable)
	FTransform GetSpawnTransformByIndex(int32 Index) const;
};
