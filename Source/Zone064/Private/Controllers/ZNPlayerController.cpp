#include "Controllers/ZNPlayerController.h"

#include "Blueprint/UserWidget.h"

void AZNPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ���� �����϶�, ���� ���� ��� PlayerController�� ������ ����
	// => ���� �� Ŭ���� PC �ܿ��� UI ���� => ���� ó�� �ؾ���
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

		// �����Ͱ� ������ �� default map�� MenuLevel�� �����س���
		// => ���� ���� �� ù ȭ���� MainMenuUI�� ��
		// => ���� �����Ϳ� �ѳ��� map�� MenuLevel�� �ƴϸ� ������
		// �� ó�� ȭ���� Main Menu, ���ķδ� ���õ� ����
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