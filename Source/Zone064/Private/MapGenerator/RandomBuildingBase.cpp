#include "MapGenerator/RandomBuildingBase.h"
#include "Components/StaticMeshComponent.h"

ARandomBuildingBase::ARandomBuildingBase()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ARandomBuildingBase::BeginPlay()
{
    Super::BeginPlay();
    GenerateStructure();
}

void ARandomBuildingBase::SetGridPosition(FIntPoint InGridPos)
{
    GridPosition = InGridPos;
}

void ARandomBuildingBase::GenerateStructure()
{
    const float UnitSize = 400.f;
    const float FloorHeight = UnitSize;

    for (int32 Z = 0; Z < Floors; ++Z)
    {
        // �Ʒ��� ��� ��ġ ��������
        bool bHasStairBelow = StairPositions.Contains(Z - 1);
        FIntPoint BelowStair;
        if (bHasStairBelow)
        {
            BelowStair = StairPositions[Z - 1];
        }

        for (int32 X = 0; X < Width; ++X)
        {
            for (int32 Y = 0; Y < Depth; ++Y)
            {
                // ������ ��� ��ġ���� �ٴ� �������� ����
                if (bHasStairBelow && X == BelowStair.X && Y == BelowStair.Y)
                {
                    continue;
                }

                FVector FloorLocation = FVector(X * UnitSize, Y * UnitSize, Z * FloorHeight);
                UStaticMeshComponent* Floor = NewObject<UStaticMeshComponent>(this);
                Floor->SetStaticMesh(FloorMesh);
                Floor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
                Floor->RegisterComponent();
                Floor->SetRelativeLocation(FloorLocation);
            }
        }

        // �ܺ� ����
        for (int32 X = 0; X < Width; ++X)
        {
            for (int32 Y = 0; Y < Depth; ++Y)
            {
                bool bEdgeX = (X == 0 || X == Width - 1);
                bool bEdgeY = (Y == 0 || Y == Depth - 1);
                if (bEdgeX || bEdgeY)
                {
                    FVector WallLocation = FVector(X * UnitSize, Y * UnitSize, Z * FloorHeight + FloorHeight / 2);
                    FRotator WallRot = bEdgeY ? FRotator(0, 90, 0) : FRotator::ZeroRotator;

                    UStaticMeshComponent* Wall = NewObject<UStaticMeshComponent>(this);
                    Wall->SetStaticMesh(WallMesh);
                    Wall->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
                    Wall->RegisterComponent();
                    Wall->SetRelativeLocation(WallLocation);
                    Wall->SetRelativeRotation(WallRot);
                }
            }
        }

        // ��� ���� (������ �� ����)
        if (Z < Floors - 1 && StairMesh)
        {
            int32 StairX = FMath::RandRange(0, Width - 1);
            int32 StairY = FMath::RandRange(0, Depth - 1);
            FIntPoint StairCell(StairX, StairY);
            StairPositions.Add(Z, StairCell);

            FVector StairLocation = FVector(StairX * UnitSize, StairY * UnitSize, Z * FloorHeight);
            FRotator StairRot = FRotator::ZeroRotator;

            UStaticMeshComponent* Stair = NewObject<UStaticMeshComponent>(this);
            Stair->SetStaticMesh(StairMesh);
            Stair->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            Stair->RegisterComponent();
            Stair->SetRelativeLocation(StairLocation);
            Stair->SetRelativeRotation(StairRot);
        }
    }

    // �Ա� �� (1�� ���� �߾�)
    if (DoorMesh)
    {
        FVector DoorLocation = FVector((Width / 2) * UnitSize, -UnitSize / 2, 0.f);
        UStaticMeshComponent* Door = NewObject<UStaticMeshComponent>(this);
        Door->SetStaticMesh(DoorMesh);
        Door->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        Door->RegisterComponent();
        Door->SetRelativeLocation(DoorLocation);
    }
}
