// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZNInventoryTestBaseItem.generated.h"

class USphereComponent;

UCLASS()
class ZONE064_API AZNInventoryTestBaseItem : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<USphereComponent> Sphere;
		
protected:
	// 인벤토리에서 아이템이 차지할 격자 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info | Dimensions")
	FIntPoint Dimensions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info | Icon")
	UMaterialInterface* Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info | Icon")
	UMaterialInterface* RotatedIcon;

	bool isRotated;

public:	
	AZNInventoryTestBaseItem();

	virtual void Tick(float DeltaTime) override;

	// Dimensions 반환 함수
	// - 인벤토리에 충분한 크기가 있는지, 아이템이 얼마나 큰지 등등에 사용
	FIntPoint GetDimensions() const;

	UMaterialInterface* GetIcon();
	UMaterialInterface* GetRotatedIcon();
	bool GetIsRotated();

	void RotateItem();

protected:
	virtual void BeginPlay() override;
	
};
