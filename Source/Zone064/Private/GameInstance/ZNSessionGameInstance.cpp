// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/ZNSessionGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UZNSessionGameInstance::Shutdown()
{
	IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface.IsValid())
	{
		if (SessionInterface->GetNamedSession(NAME_GameSession))
		{
			UE_LOG(LogTemp, Warning, TEXT("UZNSessionGameInstance::Shutdown - Destroying active session."));
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}
	Super::Shutdown();
}