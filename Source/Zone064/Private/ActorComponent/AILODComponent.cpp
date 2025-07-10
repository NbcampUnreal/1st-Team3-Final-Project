// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/AILODComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAILODComponent::UAILODComponent()
{
	// 타이머 사용 -> 틱 필요 없음
	PrimaryComponentTick.bCanEverTick = false;
}

void UAILODComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AILODComponent owner is not a Pawn."));
		return;
	}

	// 서버에서 캐싱
	if (OwnerPawn->HasAuthority())
	{
		OwnerAIController = Cast<AAIController>(OwnerPawn->GetController());
		OwnerMovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();
		OwnerMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();

		// 거리순 설정 정렬
		LODSettings.Sort([](const FAILODSetting& A, const FAILODSetting& B)
			{
				return A.DistanceThreshold > B.DistanceThreshold;
			});

		// 한 번 수동 실행후 나머진 타이머로 반복
		CheckLOD();
		GetWorld()->GetTimerManager().SetTimer(LODCheckTimerHandle, this, &UAILODComponent::CheckLOD, CheckInterval, true);

	}
}

void UAILODComponent::CheckLOD()
{
	if (!OwnerPawn || LODSettings.Num() == 0) return;

	float MinDistanceSquared = -1.0f;

	// 게임 스테이트의 플레이어 어레이 순회해서 가장 가까운 플레이어 탐색
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogTemp, Display, TEXT("[AI LOD] GameState fail"));
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState)
		{
			APawn* PlayerPawn = PlayerState->GetPawn();
			if (PlayerPawn)
			{
				const float DistanceSquared = FVector::DistSquared(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
				if (MinDistanceSquared < 0 || DistanceSquared < MinDistanceSquared)
				{
					MinDistanceSquared = DistanceSquared;
				}
			}
			
		}
	}
	
	if (MinDistanceSquared < 0) return;

	const FAILODSetting* SettingToApply = &LODSettings.Last();

	// LODSettings 순회하며 조건에 맞는 첫 번째 설정 탐색
	for (const FAILODSetting& Setting : LODSettings)
	{
		if (MinDistanceSquared > FMath::Square(Setting.DistanceThreshold))
		{
			SettingToApply = &Setting;
			break;
		}
	}

	// 현재 설정과 다를 경우에만 변경
	if (SettingToApply && SettingToApply->LODLevel != CurrentLODLevel)
	{
		ApplyLODSettings(*SettingToApply);
		CurrentLODLevel = SettingToApply->LODLevel;
		UE_LOG(LogTemp, Display, TEXT("New AI LOD Setting changed."));
	}
}

void UAILODComponent::ApplyLODSettings(const FAILODSetting& NewLODSetting)
{
	// 캐시된 컴포넌트 유효성 검사
	if (!OwnerPawn || !OwnerAIController || !OwnerMovementComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("Apply LOD Settings fail"));
		return;
	}

	// 틱 주기 항상 설정
	OwnerPawn->SetActorTickInterval(NewLODSetting.TickInterval);

	// 새로운 LOD 레벨에 따라 기능 제어
	switch (NewLODSetting.LODLevel)
	{
	case EAILODLevel::High:
		// 모든 기능 활성화
		OwnerPawn->SetActorHiddenInGame(false);
		OwnerPawn->SetActorTickEnabled(true);
		OwnerPawn->SetActorEnableCollision(true);

		if (auto Brain = OwnerAIController->GetBrainComponent())
		{
			Brain->StartLogic();
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] High : StartLogic"));
		}
		if (auto Perception = OwnerAIController->GetAIPerceptionComponent())
		{
			Perception->SetActive(true);
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] High : AIPerception active"));
		}
		OwnerMovementComponent->bUseRVOAvoidance = true;

		UE_LOG(LogTemp, Display, TEXT("[AI LOD] High : RVO true"));
		OwnerMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		break;

	case EAILODLevel::Medium:
		// RVO만 비활성화
		OwnerPawn->SetActorHiddenInGame(false);
		OwnerPawn->SetActorTickEnabled(true);
		OwnerPawn->SetActorEnableCollision(true);

		if (auto Brain = OwnerAIController->GetBrainComponent())
		{
			Brain->StartLogic();
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Medium : StartLogic"));
		}
		if (auto Perception = OwnerAIController->GetAIPerceptionComponent())
		{
			Perception->SetActive(true);
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Medium : AIPerception active"));
		}
		
		OwnerMovementComponent->bUseRVOAvoidance = false;
		UE_LOG(LogTemp, Display, TEXT("[AI LOD] Medium : RVO false"));
		OwnerMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);

		break;

	case EAILODLevel::Low:
		// 대부분 비활성화
		OwnerPawn->SetActorHiddenInGame(false);
		OwnerPawn->SetActorTickEnabled(true);
		OwnerPawn->SetActorEnableCollision(true);

		if (auto Brain = OwnerAIController->GetBrainComponent())
		{
			Brain->StartLogic();
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Low : StartLogic"));
		}
		if (auto Perception = OwnerAIController->GetAIPerceptionComponent())
		{
			Perception->SetActive(false);
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Low : AIPerception inactive"));
		}
		OwnerMovementComponent->bUseRVOAvoidance = false;
		UE_LOG(LogTemp, Display, TEXT("[AI LOD] Low : RVO false"));
		
		// 애니메이션을 단일 애셋 반복 재생으로 변경
		if (LowLOD_Animation)
		{
			OwnerMesh->SetAnimation(LowLOD_Animation);
			OwnerMesh->Play(true);
		}
		else
		{
			OwnerMesh->Stop();
		}

		break;

	case EAILODLevel::VeryLow:
		// Low보다 더 비활성화, 이동은 AI Move To 로만 가능
		OwnerPawn->SetActorHiddenInGame(true);
		OwnerPawn->SetActorTickEnabled(true);
		OwnerPawn->SetActorEnableCollision(false);

		if (auto Brain = OwnerAIController->GetBrainComponent())
		{
			Brain->StopLogic("");
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] VeryLow : StopLogic"));
		}
		if (auto Perception = OwnerAIController->GetAIPerceptionComponent())
		{
			Perception->SetActive(false);
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] VeryLow : AIPerception inactive"));
		}
		OwnerMovementComponent->bUseRVOAvoidance = false;
		UE_LOG(LogTemp, Display, TEXT("[AI LOD] VeryLow : RVO false"));

		// 애니메이션 정지
		OwnerMesh->Stop();
		break;

	case EAILODLevel::Culled:
		// 모든 기능 정지
		OwnerPawn->SetActorHiddenInGame(true);    
		OwnerPawn->SetActorEnableCollision(false); 
		OwnerPawn->SetActorTickEnabled(false);     

		if (auto Brain = OwnerAIController->GetBrainComponent())
		{
			Brain->StopLogic("");
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Culled : StopLogic"));
		}
		if (auto Perception = OwnerAIController->GetAIPerceptionComponent())
		{
			Perception->SetActive(false);
			UE_LOG(LogTemp, Display, TEXT("[AI LOD] Culled : AIPerception inactive"));
		}

		OwnerMovementComponent->bUseRVOAvoidance = false;
		// 애니메이션을 단일 애셋 반복 재생으로 변경
		OwnerMesh->Stop();
		break;
	}

	//UE_LOG(LogTemp, Log, TEXT("%s new LOD is %d"), *OwnerPawn->GetName(), static_cast<int32>(NewLODLevel));
}

