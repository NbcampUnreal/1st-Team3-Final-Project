#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h" 
#include "BuildingConstructData.generated.h"

UENUM(BlueprintType)
enum class EBuildingFace : uint8
{
	EBF_FrontFace	UMETA(DisplayName = "FrontFace"),
	EBF_BackFace	UMETA(DisplayName = "BackFace"),
	EBF_LeftFace	UMETA(DisplayName = "LeftFace"),
	EBF_RightFace	UMETA(DisplayName = "RightFace"),
	EBF_Roof		UMETA(DisplayName = "Roof"),
	EBF_Sidewalk	UMETA(DisplayName = "Sidewalk"),
	EBF_Floor		UMETA(DisplayName = "Floor")
};

USTRUCT(BlueprintType)
struct ZONE064_API FScatterData
{
	GENERATED_BODY()

	FScatterData()
	{
		BuildingFace = EBuildingFace::EBF_FrontFace;
		RandomRotation = false;
		MeshScale = FVector::OneVector;
		ScatterDensity = 0;
		BBox = nullptr;
		AllowScattering = false;
		CullingDistance = 0;
		CollisionSetting = ECollisionEnabled::QueryAndPhysics;
		LocationOffset = FVector::ZeroVector;
		RotationOffset = FRotator::ZeroRotator;
		ScaleMin = FVector::OneVector;
		ScaleMax = FVector::OneVector;
		AltitudeVariation = 0.0f;
	}

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UStaticMesh>> StaticMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuildingFace BuildingFace; // C++로 정의한 Enum 사용

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RandomRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScatterDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BBox; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AllowScattering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CullingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionEnabled::Type> CollisionSetting; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ScaleMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ScaleMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AltitudeVariation;
};

USTRUCT(BlueprintType)
struct ZONE064_API FBuildingConstructData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISMComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstanceToWorldSpace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> ScatteringHISMs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScatterData ScatterData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLineTracedScatterMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSocketScatter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> SocketScatterHISMs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UStaticMesh>> SocketScatterMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceID;

	FBuildingConstructData()
		: Transform(FTransform::Identity)
		, Mesh(nullptr)
		, MaterialInstance(nullptr)
		, HISMComponent(nullptr)
		, bInstanceToWorldSpace(false)
		, ScatteringHISMs()
		, ScatterData()
		, bIsLineTracedScatterMesh(false)
		, bIsSocketScatter(false)
		, SocketScatterHISMs()
		, SocketScatterMeshes()
		, InstanceID(0)
	{
	}
};