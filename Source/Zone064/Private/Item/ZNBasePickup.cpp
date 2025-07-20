#include "Item/ZNBasePickup.h"
#include "GameInstance/ZNBaseGameInstance.h"
#include "Data/ZNItemData.h"
#include "Engine/Engine.h"
#include "Components/MeshComponent.h"

AZNBasePickup::AZNBasePickup()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	// CollisionComponent (RootComponent)
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(50.0f);

	// Mesh Components
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	// 초기에는 둘 다 비활성화
	SkeletalMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetVisibility(false);
}

void AZNBasePickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (!ItemRowName.IsNone())
	{
		SetupMeshFromItemData();
		
		if (CachedItemData)
		{
			SetItemData(ItemRowName, 1,CachedItemData->Durability);
		}
	}
}

void AZNBasePickup::SetItemRowName(const FName& NewRowName)
{
	ItemRowName = NewRowName;
	
	// 런타임에 RowName이 변경되면 즉시 메쉬 설정
	if (!ItemRowName.IsNone())
	{
		SetupMeshFromItemData();
	}
}

void AZNBasePickup::SetupMeshFromItemData()
{
	// GameInstance에서 ItemData 가져오기
	UZNBaseGameInstance* GameInstance = Cast<UZNBaseGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	CachedItemData = GameInstance->GetItemDataFromPickup(ItemRowName);
	if (!CachedItemData) return;

	// 메쉬 설정 (SkeletalMesh 우선, 없으면 StaticMesh)
	if (CachedItemData->SkeletalMesh)
	{
		// SkeletalMesh 사용
		SkeletalMeshComponent->SetSkeletalMesh(CachedItemData->SkeletalMesh);
		SkeletalMeshComponent->SetVisibility(true);
		StaticMeshComponent->SetVisibility(false);
		ActiveMeshComponent = SkeletalMeshComponent;
	}
	else if (CachedItemData->StaticMesh)
	{
		// StaticMesh 사용
		StaticMeshComponent->SetStaticMesh(CachedItemData->StaticMesh);
		StaticMeshComponent->SetVisibility(true);
		SkeletalMeshComponent->SetVisibility(false);
		ActiveMeshComponent = StaticMeshComponent;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ZNBasePickup: No mesh found in ItemData for RowName '%s'"), *ItemRowName.ToString());
		return;
	}

	// Scale 및 Physics 적용
	if (ActiveMeshComponent)
	{
		ActiveMeshComponent->SetWorldScale3D(CachedItemData->PickupScale);
		ActiveMeshComponent->SetSimulatePhysics(true);
	}
}

void AZNBasePickup::SetItemData(const FName& RowName, int32 Quantity, int32 Durability)
{
	SetItemRowName(RowName);
	ItemQuantity = Quantity;
	ItemDurability = Durability;
}

void AZNBasePickup::GetPickupItemInfo(FPrimaryAssetId& OutItemId, int32& OutQuantity, int32& OutDurability) const
{
	OutItemId = GetItemPrimaryAssetId();
	OutQuantity = GetActualQuantity();
	OutDurability = GetActualDurability();
}

FPrimaryAssetId AZNBasePickup::GetItemPrimaryAssetId() const
{
	if (CachedItemData)
	{
		return CachedItemData->GetPrimaryAssetId();
	}
	return FPrimaryAssetId();  
}

int32 AZNBasePickup::GetActualQuantity() const
{
	return ItemQuantity > 0 ? ItemQuantity : 1;
}

int32 AZNBasePickup::GetActualDurability() const
{
	if (ItemDurability >= 0)
	{
		return ItemDurability;  // 설정된 내구도 사용
	}
	
	return -1;  // 기본값
}
