// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSystems/Types/GamePhaseTypes.h"
#include "UIManager.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	// todo: CoreHUD 제작

	/* Widget Data Table */
	TMap<EGamePhase, FName> WidgetCache;	// todo: 항목 변경해줘야 함
};
