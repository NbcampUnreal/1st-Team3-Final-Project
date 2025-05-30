// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator/ZNPlayerStart.h"
#include "components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AZNPlayerStart::AZNPlayerStart(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) // ✅ APlayerStart 생성자 명시 호출
{
    // ...
}
