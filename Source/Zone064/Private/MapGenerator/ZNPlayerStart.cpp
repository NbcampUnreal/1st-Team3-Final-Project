// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/ZNPlayerStart.h"
#include "components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AZNPlayerStart::AZNPlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	CarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Car Mesh"));
	CarMesh->SetupAttachment(RootComponent);

	for (int i = 0; i < 4; ++i)
	{
		FString Name = FString::Printf(TEXT("SpawnPoint_%d"), i);
		USceneComponent* SpawnPoint = CreateDefaultSubobject<USceneComponent>(*Name);
		SpawnPoint->SetupAttachment(CarMesh);
		SpawnPoints.Add(SpawnPoint);
	}

	if (SpawnPoints.Num() == 4)
	{
		SpawnPoints[0]->SetRelativeLocation(FVector(100, 50, 0)); 
		SpawnPoints[1]->SetRelativeLocation(FVector(100, -50, 0));
		SpawnPoints[2]->SetRelativeLocation(FVector(-100, 50, 0)); 
		SpawnPoints[3]->SetRelativeLocation(FVector(-100, -50, 0));
	}

}

FTransform AZNPlayerStart::GetSpawnTransformByIndex(int32 Index) const
{
	if (SpawnPoints.IsValidIndex(Index) && SpawnPoints[Index])
	{
		return SpawnPoints[Index]->GetComponentTransform();
	}

	return GetActorTransform(); 
}
