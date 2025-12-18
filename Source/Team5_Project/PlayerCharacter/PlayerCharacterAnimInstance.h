// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacterAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackStart);

UCLASS()
class TEAM5_PROJECT_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// PlayerCharacter가 구독할 이벤트
	FOnAttackStart OnAttackStart;

protected:
	// 애님 노티 이름이 AttackStart면 자동으로 호출됨 (AnimNotifyName 규칙)
	UFUNCTION()
	void AnimNotify_AttackStart();
};
