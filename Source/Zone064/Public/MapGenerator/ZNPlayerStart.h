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
	AZNPlayerStart();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* CarMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USceneComponent*> SpawnPoints;

	

private:

	


};
