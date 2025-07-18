
#include "MapGenerator/WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "Zone064/ObjectPoolComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BrainComponent.h"

AWaveManager::AWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetNetDormancy(ENetDormancy::DORM_Initial);

}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	GetAllTaggedActors();
}

void AWaveManager::GetAllTaggedActors()
{
	/*
	*	에디터에서 설정된 SpawnerData 순회
	*	태그와 액터에 맞춰 동적 배열 생성
	*/
	
	// 권한 체크
	if (!HasAuthority()) return;

	for (const FWaveSpawnerGroup& Group : SpawnerGroup)
	{
		if (Group.TagName == NAME_None) continue;

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), Group.TagName, FoundActors);

		if (!FoundActors.IsEmpty())
		{
			TArray<TObjectPtr<AActor>> TaggedActors;
			for (AActor* Actor : FoundActors)
			{
				TaggedActors.Add(Actor);
			}
			SpawnerMap.Add(Group.TagName, TaggedActors);
		}	
	}
}

void AWaveManager::ActivateSpawners(FName TagName)
{
	/*
	*	1.WaveInfos와 SpawnerGroup 순회
	*	2.TagName에 맞는 Spawner 목록 캐싱
	*	3.SpawnInterval 마다 반복하도록 타이머 설정
	*/
	if (!HasAuthority()) return;
	
	if (TagName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] ActivateSpawners : Invalid Tag"));
		return;
	}
	
	const FWaveSpawnerGroup* FoundGroup = SpawnerGroup.FindByPredicate([&](const FWaveSpawnerGroup& Group)
	{
		return Group.TagName == TagName;
	});

	if (!FoundGroup || FoundGroup->SpawnInterval <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] ActivateSpawners : Tag '%s' is not found or Invalid SpawnInterval"), *TagName.ToString());
		return;
	}

	if (WaveTimerHandles.Contains(TagName) && GetWorld()->GetTimerManager().IsTimerActive(WaveTimerHandles[TagName]))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] ActivateSpawners : Tag '%s' is already active."), *TagName.ToString());
	}

	FTimerHandle NewTimerHandle;
	FTimerDelegate NewTimerDelegate;
	
	NewTimerDelegate.BindUObject(this, &AWaveManager::SpawnWave, TagName);
	GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, NewTimerDelegate, FoundGroup->SpawnInterval, true, 0.0f);

	WaveTimerHandles.Add(TagName, NewTimerHandle);

	UE_LOG(LogTemp, Display, TEXT("[WaveManager] ActivateSpawners : Tag '%s' activated. Interval : %f"), *TagName.ToString(), FoundGroup->SpawnInterval);

}

void AWaveManager::SpawnWave(FName TagName)
{
	/*
	*	실제 스폰 함수
	*/
	const TArray<TObjectPtr<AActor>>* SpawnersPtr = SpawnerMap.Find(TagName);
	if (!SpawnersPtr || SpawnersPtr->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] SpawnWave : No Spawners found"));
		return;

		//if (WaveTimerHandles.Contains(TagName))
		//{
		//	GetWorld()->GetTimerManager().ClearTimer(WaveTimerHandles[TagName]);
		//	WaveTimerHandles.Remove(TagName);
		//}
		//return;
	}

	const TArray<TObjectPtr<AActor>>& Spawners = *SpawnersPtr;
	TArray<FWaveInfo> Infos;
	float TotalWeight = 0.f;
	for (const FWaveInfo& Info : WaveInfos)
	{
		if (Info.TagName == TagName)
		{
			Infos.Add(Info);
			TotalWeight += Info.SpawnChance;
		}
	}

	for (const auto& Spawner : Spawners)
	{
		if (!Spawner) continue;

		const float RandomFloat = FMath::FRandRange(0.f, TotalWeight);
		float CurrentWeight = 0.f;
		TSubclassOf<AActor> SelectedActorClass;
		for (const FWaveInfo& Info : WaveInfos)
		{
			CurrentWeight += Info.SpawnChance;
			if (RandomFloat <= CurrentWeight)
			{
				SelectedActorClass = Info.ActorToSpawn;
				break;
			}
		}

		if (SelectedActorClass)
		{
			//// 오브젝트 풀의 스폰 함수 호출 -> 오브젝트풀 코드 수정 필요 : 종류별로 스폰하도록
			//UObjectPoolComponent* ObjectPool = GetComponentByClass<UObjectPoolComponent>();

			//if (ObjectPool)
			//{
			//	FTransform SpawnTransform = FTransform(
			//		FRotator(0.f, FMath::FRandRange(0.f, 180.f), 0.f),
			//		Spawner->GetActorLocation(),
			//		FVector(1.f)
			//	);

			//	ObjectPool->SpawnPooledObject(SpawnTransform);
			//}
			//else
			//{
			//	UE_LOG(LogTemp, Warning, TEXT("[WaveManager] SpawnWave : ObjectPoolComponent is not valid."));
			//}
		}
	}
}

void AWaveManager::GetAIControllerAndStartLogic(AActor* SpawnedActor)
{
	APawn* AsPawn = Cast<APawn>(SpawnedActor);
	if (AsPawn)
	{
		AAIController* AIController = Cast<AAIController>(AsPawn->GetController());
		if (AIController)
		{
			UBrainComponent* Brain = AIController->GetBrainComponent();
			if (Brain)
			{
				Brain->StartLogic();
			}
			else UE_LOG(LogTemp, Warning, TEXT("[WaveManager] GetAIControllerAndStartLogic : Get Brain Component falied"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[WaveManager] GetAIControllerAndStartLogic : Cast to AIController failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] GetAIControllerAndStartLogic : Cast to Pawn failed"));
	}

}

void AWaveManager::NotifyRandomPlayerLocation()
{
	// ObjectPool 에서 현재 활성화된 좀비 배열이 필요함
	
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogTemp, Display, TEXT("[WaveManager] NotifyRandomPlayerLocation : GameState fail"));
		return;
	}

	// 무작위 플레이어 위치로 이동
	int32 RandomPlayerIndex = FMath::RandRange(0, GameState->PlayerArray.Num() - 1);

	if (GameState)
	{
		APlayerState* RandomPlayerState = GameState->PlayerArray[RandomPlayerIndex];
		if (RandomPlayerState)
		{
			APawn* RandomPlayerPawn = RandomPlayerState->GetPawn();
			if (/*AIPawn &&*/ RandomPlayerPawn)
			{
				/* AIPawn->GetController
				*  AIController->MoveToActor(RandomPlayerPawn, 100.f);
				*/

			}
		}
	}
}





