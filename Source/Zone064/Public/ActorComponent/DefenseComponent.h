// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DefenseComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UDefenseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDefenseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// 현재 방어력 (기본값: 0)
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	float CurrentDefense = 0.f;

	// 장비를 통해 추가하는 함수
	UFUNCTION(BlueprintCallable, Category = "Defense")
	void AddDefense(float Amount);

	// 장비를 벗을 때 차감하는 함수
	UFUNCTION(BlueprintCallable, Category = "Defense")
	void RemoveDefense(float Amount);

	// 데미지 경감 계산 함수
	UFUNCTION(BlueprintCallable, Category = "Defense")
	float ModifyIncomingDamage(float RawDamage) const;	
};
