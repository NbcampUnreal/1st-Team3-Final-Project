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
	// �κ��丮���� �������� ������ ���� ũ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info | Dimensions")
	FIntPoint Dimensions;

public:	
	AZNInventoryTestBaseItem();

	virtual void Tick(float DeltaTime) override;

	// Dimensions ��ȯ �Լ�
	// - �κ��丮�� ����� ũ�Ⱑ �ִ���, �������� �󸶳� ū�� �� ���
	FIntPoint GetDimensions() const;

protected:
	virtual void BeginPlay() override;
	
};
