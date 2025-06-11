// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "ZNBaseGameState.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API AZNBaseGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void BeginPlay() override;

	/* Getter, Setter, ... etc. */
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	EGamePhase GetCurrentGamePhase();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	FName GetCurrentMapName();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	int32 GetCurrentRepeatCount();
	void SetCurrentGamePhase(EGamePhase _GamePhase);
	void SetCurrentMapName(FName _MapName);
	void SetCurrentRepeatCount(int32 _RepeatCount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/* GameFlow Data */
	UPROPERTY(Replicated)
	EGamePhase CurrentGamePhase;
	UPROPERTY(Replicated)
	FName CurrentMapName;
	UPROPERTY(Replicated)
	int32 CurrentRepeatCount;
};
