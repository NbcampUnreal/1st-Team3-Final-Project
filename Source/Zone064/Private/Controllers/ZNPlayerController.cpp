#include "Controllers/ZNPlayerController.h"

#include "Blueprint/UserWidget.h"

void AZNPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 리슨 서버일때, 서버 측은 모든 PlayerController를 가지고 있음
	// => 서버 측 클라의 PC 외에는 UI 없음 => 예외 처리 해야함
	if (IsLocalController())
	{
		if (LobbyListUIClass)
		{
			LobbyListUIInstance = CreateWidget<UUserWidget>(this, LobbyListUIClass);
		}

		if (CreateGameUIClass)
		{
			CreateGameUIInstance = CreateWidget<UUserWidget>(this, CreateGameUIClass);
		}

		// 에디터가 켜졌을 때 default map을 MenuLevel로 지정해놨음
		// => 게임 실행 시 첫 화면이 MainMenuUI가 뜸
		// => 만약 에디터에 켜놓은 map이 MenuLevel이 아니면 무시함
		// 맨 처음 화면은 Main Menu, 이후로는 무시될 거임
		FString CurrentMapName = GetWorld()->GetMapName();
		if (CurrentMapName.Contains("LobbyList"))
		{
			LobbyListUIInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
		else
		{
			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}
	}
}