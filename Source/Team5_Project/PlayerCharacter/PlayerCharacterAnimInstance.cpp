// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/PlayerCharacterAnimInstance.h"

void UPlayerCharacterAnimInstance::AnimNotify_AttackStart()
{
	OnAttackStart.Broadcast();
}

void UPlayerCharacterAnimInstance::AnimNotify_SFX_Player_Footstep1()
{
	OnSFX_Player_Footstep1.Broadcast();
}

void UPlayerCharacterAnimInstance::AnimNotify_SFX_Player_Footstep2()
{
	OnSFX_Player_Footstep2.Broadcast();
}