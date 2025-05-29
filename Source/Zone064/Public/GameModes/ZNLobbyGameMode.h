// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/ZNBaseGameMode.h"
#include "ZNLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API AZNLobbyGameMode : public AZNBaseGameMode
{
	GENERATED_BODY()
	
public:
	AZNLobbyGameMode();

	// 세션에 참가 하였을 때 발생
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// 세션을 나갔을 때 발생
	virtual void Logout(AController* Exiting) override;
};
