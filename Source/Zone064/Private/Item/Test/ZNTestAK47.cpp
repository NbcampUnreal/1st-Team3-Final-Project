// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Test/ZNTestAK47.h"

AZNTestAK47::AZNTestAK47()
{
	Dimensions = FIntPoint(5, 2);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ItemIcon(TEXT("/Game/Items/Test/Icons/Materials/MI_AK47"));
	if (ItemIcon.Succeeded())
	{
		Icon = ItemIcon.Object;
	}
}
