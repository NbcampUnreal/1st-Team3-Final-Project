#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZNPlayerController.generated.h"

UCLASS()
class ZONE064_API AZNPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> LobbyListUIClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HUD")
	TObjectPtr<UUserWidget> LobbyListUIInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> CreateGameUIClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HUD")
	TObjectPtr<UUserWidget> CreateGameUIInstance;

protected:
	virtual void BeginPlay() override;

};
