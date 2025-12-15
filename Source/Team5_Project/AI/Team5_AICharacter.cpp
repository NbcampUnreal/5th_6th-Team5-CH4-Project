#include "AI/Team5_AICharacter.h"

#include "Team5_DamageTakenComponent.h"

ATeam5_AICharacter::ATeam5_AICharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DamageTakenComponent = CreateDefaultSubobject<UTeam5_DamageTakenComponent>("DamageTakenComponent");
}

void ATeam5_AICharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ATeam5_AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
