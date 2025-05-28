// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "UIManager.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ZONE064_API UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	/* Widget Data Table */
	TMap<EGamePhase, TArray<TSoftClassPtr<UUserWidget>>> WidgetCache;
};
