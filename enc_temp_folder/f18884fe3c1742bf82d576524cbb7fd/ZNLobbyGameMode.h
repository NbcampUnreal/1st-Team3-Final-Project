// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ZNLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API AZNLobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AZNLobbyGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
