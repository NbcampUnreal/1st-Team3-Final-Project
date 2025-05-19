// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/ZNLobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintFunctionLibrary/ZNSessionLibrary.h"
#include "Controllers/ZNPlayerController.h"

AZNLobbyGameMode::AZNLobbyGameMode()
{
    PlayerControllerClass = AZNPlayerController::StaticClass();
}

void AZNLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("인원 +1"));
    }

    UZNSessionLibrary::UpdatePlayerCountInSession(this, +1);
}

void AZNLobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("인원 -1"));
    }

    UZNSessionLibrary::UpdatePlayerCountInSession(this, -1);
}