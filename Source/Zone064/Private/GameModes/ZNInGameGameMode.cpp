#include "GameModes/ZNInGameGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintFunctionLibrary/ZNSessionLibrary.h"
#include "Controllers/ZNPlayerController.h"

AZNInGameGameMode::AZNInGameGameMode()
{
	PlayerControllerClass = AZNPlayerController::StaticClass();
}

void AZNInGameGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("인원 +1"));
    }

    UZNSessionLibrary::UpdatePlayerCountInSession(this, +1);
}

void AZNInGameGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("인원 -1"));
    }

    UZNSessionLibrary::UpdatePlayerCountInSession(this, -1);
}