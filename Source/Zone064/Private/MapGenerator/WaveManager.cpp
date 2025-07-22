
#include "MapGenerator/WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "Zone064/ObjectPoolComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

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
	if (HasAuthority())
	{
		ObjectPoolComponent = this->FindComponentByClass<UObjectPoolComponent>();
	}
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

void AWaveManager::StartWave(FName TagName, int32 NumberToSpawn, float PreparationTime)
{
	if (!HasAuthority()) return;

	if (TagName == NAME_None || NumberToSpawn <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] StartWave: Invalid TagName or NumberToSpawn."))
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[WaveManager] Preparing wave for tag '%s'. Spawning %d actors in %f seconds."), *TagName.ToString(), NumberToSpawn, PreparationTime);

	FTimerHandle WaveStartTimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AWaveManager::ExecuteWaveSpawn, TagName, NumberToSpawn);
	
	GetWorld()->GetTimerManager().SetTimer(WaveStartTimerHandle, TimerDelegate, PreparationTime, false);
}


void AWaveManager::ExecuteWaveSpawn(FName TagName, int32 NumberToSpawn)
{
	if (!HasAuthority() || !ObjectPoolComponent) return;

	const TArray<TObjectPtr<AActor>>* SpawnersPtr = SpawnerMap.Find(TagName);
	if (!SpawnersPtr || SpawnersPtr->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] ExecuteWaveSpawn: No spawners found for tag '%s'."), *TagName.ToString());
		return;
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

	if (Infos.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] ExecuteWaveSpawn: No wave info found for tag '%s'."), *TagName.ToString());
		return;
	}

	int32 SpawnedCount = 0;
	for (int32 i = 0; i < NumberToSpawn; ++i)
	{
		const TObjectPtr<AActor> RandomSpawner = Spawners[FMath::RandRange(0, Spawners.Num() - 1)];
		if (!RandomSpawner) continue;

		const float RandomFloat = FMath::FRandRange(0.f, TotalWeight);
		float CurrentWeight = 0.f;
		TSubclassOf<AActor> SelectedActorClass;
		for (const FWaveInfo& Info : Infos)
		{
			CurrentWeight += Info.SpawnChance;
			if (RandomFloat <= CurrentWeight)
			{
				SelectedActorClass = Info.ActorToSpawn;
				break;
			}
		}

		FNavLocation RandomNavLocation;
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		bool bFoundLocation = false;
		if (NavSys)
		{
			bFoundLocation = NavSys->GetRandomPointInNavigableRadius(RandomSpawner->GetActorLocation(), SpawnSpreadRadius, RandomNavLocation);
		}

		if (SelectedActorClass && bFoundLocation)
		{
			// 스폰할 액터의 기본 콜리전 컴포넌트 높이를 가져옴
			float ActorHalfHeight = 50.f;
			if (ACharacter* DefaultCharacter = SelectedActorClass->GetDefaultObject<ACharacter>())
			{
				if (UCapsuleComponent* Capsule = DefaultCharacter->GetCapsuleComponent())
				{
					ActorHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}
			}

			FVector SpawnLocation = RandomNavLocation.Location + FVector(0.f, 0.f, ActorHalfHeight);
			FTransform SpawnTransform = FTransform(
				FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f),
				SpawnLocation,
				FVector(1.f)
			);
			
			AActor* SpawnedActor = ObjectPoolComponent->SpawnPooledObject(SelectedActorClass, SpawnTransform);
			if(SpawnedActor) 
			{
				GetAIControllerAndStartLogic(SpawnedActor);
				SpawnedCount++;
			}
		}
		else
		{
			// 네비게이션 위치를 찾지 못했을 때
			FTransform SpawnTransform = FTransform(
				FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f),
				RandomSpawner->GetActorLocation(),
				FVector(1.f)
			);
			AActor* SpawnedActor = ObjectPoolComponent->SpawnPooledObject(SelectedActorClass, SpawnTransform);
			if(SpawnedActor) 
			{
				GetAIControllerAndStartLogic(SpawnedActor);
				SpawnedCount++;
			}
		}
	}

	// 모든 활성화된 AI에게 이동 명령
	for (auto& PoolPair : ObjectPoolComponent->PoolMap)
	{
		FObjectPool& Pool = PoolPair.Value;
		for (AActor* Actor : Pool.InUseActors)
		{
			APawn* AIPawn = Cast<APawn>(Actor);
			if (AIPawn)
			{
				AAIController* AIController = Cast<AAIController>(AIPawn->GetController());
				if (AIController)
				{
					// 각 AI마다 다른 무작위 플레이어를 목표로 설정
					APawn* TargetPlayerPawn = GetRandomPlayerPawn();
					if (TargetPlayerPawn)
					{
						AIController->MoveToActor(TargetPlayerPawn, 100.f);
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Display, TEXT("[WaveManager] Wave executed for tag '%s'. %d actors spawned. All in-use actors are now moving to different random targets."), *TagName.ToString(), SpawnedCount);
}

void AWaveManager::MoveWave()
{

}

int32 AWaveManager::GetActiveActorCount() const
{
	if (!ObjectPoolComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveManager] GetActiveActorCount: ObjectPoolComponent is not valid."));
		return 0;
	}
	
	int32 TotalInUseActors = 0;
	for (const auto& PoolPair : ObjectPoolComponent->PoolMap)
	{
		TotalInUseActors += PoolPair.Value.InUseActors.Num();
	}

	return TotalInUseActors;
}

bool AWaveManager::CanSpawnMoreActors(int32 Threshold) const
{
	return GetActiveActorCount() < Threshold;
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


APawn* AWaveManager::GetRandomPlayerPawn()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogTemp, Display, TEXT("[WaveManager] GetRandomPlayerPawn : GameState fail"));
		return nullptr;
	}

	int32 RandomPlayerIndex = FMath::RandRange(0, GameState->PlayerArray.Num() - 1);

	if (GameState)
	{
		APlayerState* RandomPlayerState = GameState->PlayerArray[RandomPlayerIndex];
		if (RandomPlayerState)
		{
			APawn* RandomPlayerPawn = RandomPlayerState->GetPawn();
			if (RandomPlayerPawn)
			{
				return RandomPlayerPawn;
			}
		}
	}
	
	return nullptr;
}





