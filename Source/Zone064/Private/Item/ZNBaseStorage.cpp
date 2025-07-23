// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ZNBaseStorage.h"

// Sets default values
AZNBaseStorage::AZNBaseStorage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AZNBaseStorage::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZNBaseStorage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

