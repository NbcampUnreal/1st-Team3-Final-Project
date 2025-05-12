#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ZNInGameGameMode.generated.h"


UCLASS()
class ZONE064_API AZNInGameGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AZNInGameGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
