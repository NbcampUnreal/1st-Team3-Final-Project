// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Inventory/ZNInventoryTestCharacter.h"
#include "Character/Inventory/ZNInventoryComponent.h"
#include "Item/Test/ZNInventoryTestBaseItem.h"
#include "UI/Inventory/ZNInventoryGridWidget.h"
#include "Controllers/ZNPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"

AZNInventoryTestCharacter::AZNInventoryTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

    InventoryComponent = CreateDefaultSubobject<UZNInventoryComponent>(TEXT("InventoryComponent"));

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AZNInventoryTestCharacter::OnBeginOverlap);
}

void AZNInventoryTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    CurrentPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (CurrentPlayerController)
    {
        if (InventoryWidgetClass)
        {
            InventoryWidgetInstance = CreateWidget(GetWorld(), InventoryWidgetClass);
            InventoryWidgetInstance->SetOwningPlayer(CurrentPlayerController);

            // 3
            InventoryWidgetInstance->AddToViewport();
            InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // 2
    // 근데 왜 캐릭터에서 초기화 하는거지? 인벤토리 컴포넌트의 생성자나 BeginPlay에서 하면 되는 것 아닌가?
    InventoryComponent->Items.SetNum(InventoryComponent->Columns * InventoryComponent->Rows);
}

void AZNInventoryTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZNInventoryTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (AZNPlayerController* PlayerController = Cast<AZNPlayerController>(GetController()))
        {
            EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &AZNInventoryTestCharacter::ToggleInventory);
        }
    }
}

void AZNInventoryTestCharacter::ToggleInventory()
{
    // if(InventoryWidgetInstance->IsInViewport())
    // {
    //     InventoryWidgetInstance->RemoveFromParent();
    //     CurrentPlayerController->SetShowMouseCursor(false);
    //     CurrentPlayerController->SetInputMode(FInputModeGameOnly());
    // }
    // else
    // {
    //     InventoryWidgetInstance->AddToViewport();
    //     CurrentPlayerController->SetShowMouseCursor(true);
    //     CurrentPlayerController->SetInputMode(FInputModeGameAndUI());
    // }

    // 3 Delete Upper Code
    if (InventoryWidgetInstance->GetVisibility() == ESlateVisibility::Collapsed)
    {
        InventoryWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        CurrentPlayerController->SetInputMode(FInputModeGameAndUI());
        CurrentPlayerController->SetShowMouseCursor(true);
    }
    else
    {
        InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
        CurrentPlayerController->SetInputMode(FInputModeGameOnly());
        CurrentPlayerController->SetShowMouseCursor(false);
    }
}

void AZNInventoryTestCharacter::OnBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ItemToAdd = OtherActor;

    InventoryComponent->InventoryGridWidgetReference->bDropped = false;

    AZNInventoryTestBaseItem* Item = Cast<AZNInventoryTestBaseItem>(OtherActor);
    if (Item)
    {
        if (InventoryComponent->TryAddItem(Item))
        {
            // 리플리케이션 필요
            OtherActor->Destroy();
        }
    }
}