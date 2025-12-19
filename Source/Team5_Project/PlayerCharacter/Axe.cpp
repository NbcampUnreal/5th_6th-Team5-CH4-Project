#include "PlayerCharacter/Axe.h"

AAxe::AAxe()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Axe_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Axe"));
	Axe_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Axe_Mesh);
}

void AAxe::BeginPlay()
{
	Super::BeginPlay();
}
