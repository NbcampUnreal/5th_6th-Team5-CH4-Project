#include "AI/Team5_AICharacter.h"

ATeam5_AICharacter::ATeam5_AICharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AI_HP =	AI_MaxHP;
}

void ATeam5_AICharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATeam5_AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
