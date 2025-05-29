// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZNInventoryTestCharacter.generated.h"

class UZNInventoryComponent;
class UInputAction;

UCLASS()
class ZONE064_API AZNInventoryTestCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivatedAccess = "true"))
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;
	TObjectPtr<UUserWidget> InventoryWidgetInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ItemWidgetClass;
	TObjectPtr<UUserWidget> ItemWidgetInstance;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UZNInventoryComponent> InventoryComponent;

	AActor* ItemToAdd;

protected:
	// 내부적으로 캐릭터는 PlayerController를 가지고 있음
	// 그냥 혹시 몰라서 변수 하나 더 만들어서 사용하는 듯
	TObjectPtr<APlayerController> CurrentPlayerController;

public:
	AZNInventoryTestCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ToggleInventory();

};
