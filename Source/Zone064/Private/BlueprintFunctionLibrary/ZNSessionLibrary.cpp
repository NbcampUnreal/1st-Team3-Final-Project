// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibrary/ZNSessionLibrary.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameInstance/ZNSessionGameInstance.h"
#include "GameFramework/GameStateBase.h"

// 시간을 이용한 고유 세션 이름 생성
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

// 세션에 대한 설정후 세션 생성
bool UZNSessionLibrary::CreateFullSession(UObject* WorldContextObject, int32 MaxPlayers, const FString& GameName)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return false;

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return false;

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = MaxPlayers;     // 최대 인원수
    Settings.bShouldAdvertise = true;               // 세션을 온라인 서비스로 사용
    Settings.bUsesPresence = true;                  // Presence 기반 세션
    Settings.bAllowJoinViaPresence = true;          // 친구의 세션에 참여
    Settings.bAllowJoinInProgress = true;           // 진행 도중 참여
    Settings.bAllowInvites = true;                  // 친구 초대 가능
    Settings.bIsLANMatch = false;                   // LAN 세션 아님

    Settings.bUseLobbiesIfAvailable = true;             // Lobby 시스템 사용
    Settings.bUseLobbiesVoiceChatIfAvailable = true;    // 보이스챗 사용
    Settings.bAllowJoinViaPresenceFriendsOnly = false;  // 친구만 참가 X
    Settings.bIsDedicated = false;                      // 전용 서버 아님
    Settings.bUsesStats = false;                        // 스탯 비사용
    Settings.bAntiCheatProtected = false;               // 안티치트 미사용

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

// 세션 이름을 통해 세션에 참가하는 함수
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

// 불필요한 세션에 대한 연결을 끊어주는 함수
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

// 세션 이름 반환
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

// 전체 세션 목록에서 불필요한 세션을 제거후 반환
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

// 인원수 UI에 관한 업데이트 함수
FText UZNSessionLibrary::GetFormattedSessionPlayerCount(const FBlueprintSessionResult& SessionResult)
{
    int32 MaxPlayers = SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections;
    int32 CurrentPlayers = 0;

    SessionResult.OnlineResult.Session.SessionSettings.Get(FName("CURRENT_PLAYERS"), CurrentPlayers);

    return FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers));
}

// 세션의 인원수를 업데이트하는 함수
void UZNSessionLibrary::UpdatePlayerCountInSession(UObject* WorldContextObject, int32 Delta)
{
    if (!WorldContextObject) return;

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(GetCurrentSessionName(WorldContextObject));
    if (!Session) return;

    int32 CurrentPlayers = 1;
    Session->SessionSettings.Get(FName("CURRENT_PLAYERS"), CurrentPlayers);

    CurrentPlayers = FMath::Clamp(CurrentPlayers + Delta, 0, Session->SessionSettings.NumPublicConnections);
    Session->SessionSettings.Set(FName("CURRENT_PLAYERS"), CurrentPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    SessionInterface->UpdateSession(GetCurrentSessionName(WorldContextObject), Session->SessionSettings, true);
}

// 타이머를 이용하여 연속 클릭 방지 함수
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