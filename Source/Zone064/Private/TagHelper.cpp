// Fill out your copyright notice in the Description page of Project Settings.

#include "TagHelper.h"
#include "GameplayTagsManager.h"


FGameplayTag UTagHelper::NameToGameplayTag(FName TagName)
{
	return UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
}