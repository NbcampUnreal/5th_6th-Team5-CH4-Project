// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/PlayerCharacterAnimInstance.h"

void UPlayerCharacterAnimInstance::AnimNotify_AttackStart()
{
	OnAttackStart.Broadcast();
}
