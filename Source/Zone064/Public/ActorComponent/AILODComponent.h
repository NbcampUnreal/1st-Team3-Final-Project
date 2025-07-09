// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AILODComponent.generated.h"

UENUM(BlueprintType)
enum class EAILODLevel : uint8
{
	High,
	Medium,
	Low,
	Culled
};

USTRUCT(BlueprintType)
struct FAILODSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD")
	float DistanceThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD")
	float TickInterval = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UAILODComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAILODComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD", meta = (TitleProperty = "DistanceThreshold"))
	TArray<FAILODSetting> LODSettings;

	UFUNCTION()
	void ApplyLODSettings(EAILODLevel NewLODLevel);

private:
	EAILODLevel CurrentLODLevel = EAILODLevel::High;
	
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
};
