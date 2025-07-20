#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "ZNBasePickup.generated.h"

class UZNItemData;

UCLASS()
class ZONE064_API AZNBasePickup : public AActor
{
	GENERATED_BODY()

public:
	AZNBasePickup();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ItemRowName, Category = "Pickup")
	FName ItemRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Pickup")
	int32 ItemQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Pickup")
	int32 ItemDurability = -1;  // -1이면 ItemData의 기본 내구도 사용

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ItemRowName();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;
	
	UPROPERTY()
	TObjectPtr<UZNItemData> CachedItemData = nullptr;

	
public:
	// 런타임에 RowName 설정 (드롭 시 사용)
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetItemRowName(const FName& NewRowName);

	// Blueprint에서 ItemData 접근용
	UFUNCTION(BlueprintPure, Category = "Pickup")
	UZNItemData* GetCachedItemData() const { return CachedItemData; }

	// 아이템 월드에 스폰 시 한번에 설정
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetItemData(const FName& RowName, int32 Quantity, int32 Durability);

	// Interaction용 - AddItem에 바로 사용할 값들 반환
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void GetPickupItemInfo(FPrimaryAssetId& OutItemId, int32& OutQuantity, int32& OutDurability) const;
	
	// Interaction 시 바로 PrimaryAssetId 반환
	UFUNCTION(BlueprintPure, Category = "Pickup")
	FPrimaryAssetId GetItemPrimaryAssetId() const;
	
	// 실제 사용할 수량/내구도 계산하여 반환
	UFUNCTION(BlueprintPure, Category = "Pickup")
	int32 GetActualQuantity() const;

	UFUNCTION(BlueprintPure, Category = "Pickup")
	int32 GetActualDurability() const;

private:
	// 현재 활성화된 메쉬 컴포넌트
	TObjectPtr<UMeshComponent> ActiveMeshComponent = nullptr;
	
	void SetupMeshFromItemData();
};
