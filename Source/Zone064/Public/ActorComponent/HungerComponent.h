// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HungerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UHungerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHungerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 현재 맵 이름 저장
	FString CachedMapName;

	// 예외 맵 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunger")
	TArray<FString> IgnoreMaps;

public:	
	// 현재 허기 수치
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Hunger, BlueprintReadWrite, Category="Hunger")
	float Hunger = 100.f;

	// 최대 허기 수치
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category="Hunger")
	float MaxHunger = 100.f;

	// 허기 자연 감소 속도 (초당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hunger")
	float HungerDecayRate = .1f;

	// 감소를 위한 Tick 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hunger")
	bool bAutoDecrease = true;

	// 허기 감소
	UFUNCTION(BlueprintCallable, Category="Hunger")
	void DecreaseHunger(float Amount);

	// 허기 회복
	UFUNCTION(BlueprintCallable, Category="Hunger")
	void IncreaseHunger(float Amount);

	// 현재 비율 (UI용)
	UFUNCTION(BlueprintCallable, Category="Hunger")
	float GetHungerPercent() const;

	UFUNCTION(BlueprintCallable, Category="Hunger")
	void TriggerTickHungerTimer(bool IsDead);

	// 현재 허기 수치 반환
	UFUNCTION()
	void OnRep_Hunger();

private:
	FTimerHandle HungerTimerHandle;
	void TickHunger();

		
};
