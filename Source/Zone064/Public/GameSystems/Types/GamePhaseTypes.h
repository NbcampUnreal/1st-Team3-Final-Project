// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GamePhaseTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Title				UMETA(DisplayName = "Title"),				// ���� ���� ȭ��
	Menu				UMETA(DisplayName = "Menu"),				// ���� ����/���� ȭ�� (Steam ����)
	Lobby				UMETA(DisplayName = "Lobby"),				// ���� ���� (�÷��̾� �غ�, ���� ����)
	Departure			UMETA(DisplayName = "Departure"),			// ��� �غ� ���� (�ó׸�ƽ ��)
	Driving				UMETA(DisplayName = "Driving"),				// ������ �̵� (�̺�Ʈ �߻�!)			-> 4ȸ �ݺ� ��������
	InGame				UMETA(DisplayName = "InGame"),				// ����, �Ĺ�
	Camping				UMETA(DisplayName = "Camping"),				// ���� �ð� (�̺�Ʈ �߻�!, ũ������ ��)
	Voting				UMETA(DisplayName = "Voting"),				// ���� �̵� ���� ����				-> 4ȸ �ݺ� ��������
	Defense				UMETA(DisplayName = "Defense"),				// ���� ���潺
	Ending				UMETA(DisplayName = "Ending"),				// ���� ����, ���� ��� ����
	ReturnToTitle		UMETA(DisplayName = "ReturnToTitle"),		// Title�� ���ư���
};

UCLASS()
class ZONE064_API UGamePhaseTypes : public UObject
{
	GENERATED_BODY()
	
};
