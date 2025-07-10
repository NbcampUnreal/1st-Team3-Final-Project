// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimationAsset.h"
#include "AILODComponent.generated.h"

class AAIController;
class UCharacterMovementComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EAILODLevel : uint8
{
	High,
	Medium,
	Low,
	Culled
};

USTRUCT(BlueprintType)
struct FAILODSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD")
	EAILODLevel LODLevel = EAILODLevel::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD")
	float DistanceThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILOD")
	float TickInterval = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZONE064_API UAILODComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAILODComponent();

protected:
	virtual void BeginPlay() override;

public:	
	void CheckLOD();

protected:
	// LOD 설정 배열 -> 에디터에서 조절
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD")
	TArray<FAILODSetting> LODSettings;

	// 몇 초마다 거리를 검사할 것인가?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float CheckInterval = 0.5f;

	UFUNCTION()
	void ApplyLODSettings(const FAILODSetting& NewLODLevel);

	// 검색할 플레이어 캐릭터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI LOD|Animation")
	UAnimationAsset* LowLOD_Animation;


private:
	
	// 타이머 핸들
	FTimerHandle LODCheckTimerHandle;
	
	// 현재 적용된 LOD 레벨
	EAILODLevel CurrentLODLevel = EAILODLevel::High;
	
	// 컴포넌트 소유 폰과 폰의 컨트롤러, 무브먼트 컴포넌트
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;
	UPROPERTY()
	TObjectPtr<AAIController> OwnerAIController;
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> OwnerMesh;


};
