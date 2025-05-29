#include "GameModes/ZNMenuGameMode.h"

#include "Controllers/ZNPlayerController.h"

AZNMenuGameMode::AZNMenuGameMode()
{
	PlayerControllerClass = AZNPlayerController::StaticClass();
}
