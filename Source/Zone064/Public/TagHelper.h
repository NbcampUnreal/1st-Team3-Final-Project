// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "TagHelper.generated.h"

/**
 * 
 */
UCLASS()
class ZONE064_API UTagHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Gameplay Tags")
	static FGameplayTag NameToGameplayTag(FName TagName);
	
};
