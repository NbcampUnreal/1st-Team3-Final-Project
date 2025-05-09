#include "GameModes/ZNInGameGameMode.h"

#include "Controllers/ZNPlayerController.h"

AZNInGameGameMode::AZNInGameGameMode()
{
	PlayerControllerClass = AZNPlayerController::StaticClass();
}
