// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibrary/ZNSessionLibrary.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameModes/ZNSessionGameInstance.h"
#include "GameFramework/GameStateBase.h"

FString UZNSessionLibrary::GenerateUniqueSessionName(UObject* WorldContextObject)
{
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d%H%M%S"));
    FString NameString = FString::Printf(TEXT("Game_%s"), *Timestamp);
    FName SessionName = FName(*NameString);

    if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
    {
        if (UZNSessionGameInstance* GI = World->GetGameInstance<UZNSessionGameInstance>())
        {
            GI->CurrentSessionName = SessionName;
        }
    }

    return NameString;
}

void UZNSessionLibrary::SafeDestroySession(UObject* WorldContextObject)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (!Session.IsValid()) return;

    if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
    {
        if (UZNSessionGameInstance* GI = World->GetGameInstance<UZNSessionGameInstance>())
        {
            if (Session->GetNamedSession(GI->CurrentSessionName))
            {
                Session->DestroySession(GI->CurrentSessionName);
            }
        }
    }
}

FName UZNSessionLibrary::GetCurrentSessionName(UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
    {
        if (UZNSessionGameInstance* GI = World->GetGameInstance<UZNSessionGameInstance>())
        {
            return GI->CurrentSessionName;
        }
    }

    return NAME_None;
}

TArray<FBlueprintSessionResult> UZNSessionLibrary::FilterValidSessions(const TArray<FBlueprintSessionResult>& SessionResults)
{
    TArray<FBlueprintSessionResult> ValidResults;

    for (const FBlueprintSessionResult& SessionResult : SessionResults)
    {
        const FOnlineSessionSearchResult& NativeResult = SessionResult.OnlineResult;
        const auto& Session = NativeResult.Session;

        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        TSharedPtr<const FUniqueNetId> MyUniqueNetId;

        if (Subsystem)
        {
            IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
            if (IdentityInterface.IsValid())
            {
                MyUniqueNetId = IdentityInterface->GetUniquePlayerId(0);
            }
        }

        if (!NativeResult.IsValid())
        {
            continue;
        }

        if (Session.OwningUserName.IsEmpty())
        {
            continue;
        }

        int32 MaxPlayers = Session.SessionSettings.NumPublicConnections;
        int32 OpenPlayers = Session.NumOpenPublicConnections;
        int32 CurrentPlayers = MaxPlayers - OpenPlayers;
        if (MaxPlayers <= 0 || CurrentPlayers < 0 || CurrentPlayers > MaxPlayers)
        {
            continue;
        }

        FString GameName;
        if (!Session.SessionSettings.Get(TEXT("GameName"), GameName) || GameName.IsEmpty())
        {
            continue;
        }

        if (MyUniqueNetId.IsValid() &&
            NativeResult.Session.OwningUserId.IsValid() &&
            *NativeResult.Session.OwningUserId == *MyUniqueNetId)
        {
            continue;
        }

        ValidResults.Add(SessionResult);
    }

    return ValidResults;
}

bool UZNSessionLibrary::CreateFullSession(UObject* WorldContextObject, int32 MaxPlayers, const FString& GameName)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return false;

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return false;

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = MaxPlayers;
    Settings.bShouldAdvertise = true;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinViaPresence = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowInvites = true;
    Settings.bIsLANMatch = false;

    Settings.bUseLobbiesIfAvailable = true;
    Settings.bUseLobbiesVoiceChatIfAvailable = true;
    Settings.bAllowJoinViaPresenceFriendsOnly = false;
    Settings.bIsDedicated = false;
    Settings.bUsesStats = false;
    Settings.bAntiCheatProtected = false;

    Settings.Set(TEXT("GameName"), GameName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    Settings.Set(FName("CURRENT_PLAYERS"), 1, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (ULocalPlayer* LP = PC->GetLocalPlayer())
            {
                FUniqueNetIdRepl NetID = LP->GetPreferredUniqueNetId();
                if (NetID.IsValid())
                {
                    return SessionInterface->CreateSession(*NetID, GetCurrentSessionName(WorldContextObject), Settings);
                }
            }
        }
    }

    return false;
}

bool UZNSessionLibrary::JoinNamedSession(UObject* WorldContextObject, const FBlueprintSessionResult& SearchResult)
{
    if (!SearchResult.OnlineResult.IsValid())
    {
        return false;
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return false;

    IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
    if (!Sessions.IsValid()) return false;

    FName SessionName = NAME_GameSession;
    if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
    {
        if (UZNSessionGameInstance* GI = World->GetGameInstance<UZNSessionGameInstance>())
        {
            SessionName = GetCurrentSessionName(WorldContextObject);
        }

        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            FUniqueNetIdRepl NetID = PC->GetLocalPlayer()->GetPreferredUniqueNetId();
            if (NetID.IsValid())
            {
                Sessions->AddOnJoinSessionCompleteDelegate_Handle(
                    FOnJoinSessionCompleteDelegate::CreateLambda([=](FName JoinedName, EOnJoinSessionCompleteResult::Type Result)
                        {
                            if (Result == EOnJoinSessionCompleteResult::Success)
                            {
                                FString ConnectString;
                                if (Sessions->GetResolvedConnectString(JoinedName, ConnectString))
                                {
                                    PC->ClientTravel(ConnectString, TRAVEL_Absolute);
                                }
                            }
                        })
                );

                return Sessions->JoinSession(*NetID, SessionName, SearchResult.OnlineResult);
            }
        }
    }

    return false;
}

void UZNSessionLibrary::TemporarilyDisableButton(UButton* TargetButton, float DisableDuration)
{
    if (!TargetButton || DisableDuration <= 0.f) return;

    TargetButton->SetIsEnabled(false);

    if (UWorld* World = TargetButton->GetWorld())
    {
        TWeakObjectPtr<UButton> WeakButton = TargetButton;

        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda([WeakButton]()
            {
                if (WeakButton.IsValid())
                {
                    WeakButton->SetIsEnabled(true);
                }
            });

        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, TimerCallback, DisableDuration, false);
    }
}

FText UZNSessionLibrary::GetFormattedSessionPlayerCount(const FBlueprintSessionResult& SessionResult)
{
    int32 MaxPlayers = SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections;
    int32 CurrentPlayers = 0;

    SessionResult.OnlineResult.Session.SessionSettings.Get(FName("CURRENT_PLAYERS"), CurrentPlayers);

    return FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers));
}

void UZNSessionLibrary::UpdatePlayerCountInSession(UObject* WorldContextObject, int32 Delta)
{
    if (!WorldContextObject) return;

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!Session) return;

    int32 CurrentPlayers = 1;
    Session->SessionSettings.Get(FName("CURRENT_PLAYERS"), CurrentPlayers);

    CurrentPlayers = FMath::Clamp(CurrentPlayers + Delta, 0, Session->SessionSettings.NumPublicConnections);
    Session->SessionSettings.Set(FName("CURRENT_PLAYERS"), CurrentPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->UpdateSession(NAME_GameSession, Session->SessionSettings, true);
}