// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/ZNSessionGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UZNSessionGameInstance::Init()
{
	Super::Init();
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UZNSessionGameInstance::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UZNSessionGameInstance::HandleTravelFailure);
	}
	OnInit();
}

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

void UZNSessionGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const bool bIsServer = World && (World->GetNetMode() != NM_Client);
	OnNetworkError(FailureType, bIsServer);
}

void UZNSessionGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	OnTravelError(FailureType);
}