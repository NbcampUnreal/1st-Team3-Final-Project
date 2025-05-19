// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Test/ZNInventoryTestBaseItem.h"

#include "Components/SphereComponent.h"

AZNInventoryTestBaseItem::AZNInventoryTestBaseItem()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(Mesh);
}

void AZNInventoryTestBaseItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZNInventoryTestBaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FIntPoint AZNInventoryTestBaseItem::GetDimensions() const
{
	return Dimensions;
}

UMaterialInterface* AZNInventoryTestBaseItem::GetIcon()
{
	return Icon;
}