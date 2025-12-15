#include "AI/Team5_DamageTakenComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Team5_AICharacter.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter/PlayerCharacter.h"

UTeam5_DamageTakenComponent::UTeam5_DamageTakenComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsAlive = true;
}

void UTeam5_DamageTakenComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTeam5_DamageTakenComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                AController* InstigatedBy, AActor* DamageCauser)
{
	//현재체력에서 데미지를 깎음
	CurrentHP -= Damage;
	//체력을 0미만으로 내려가지 않음
	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);
	//현재 체력이 0보다 낮거나 같을 때
	if (CurrentHP <= 0.0f)
	{
		if (bIsAlive == false)
		{
			return;
		}
		bIsAlive = false;
		//Player죽음
		if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
		{
			PlayerController->StopMovement();

			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner()))
			{
				PlayerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PlayerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PlayerCharacter->SetLifeSpan(5.f);
			}
		}
	}
	//AI죽음
	if (AAIController* AIController = Cast<AAIController>(GetOwner()))
	{
		bIsAlive = false;
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Dead"));
		}
		AIController->StopMovement();
		if (ATeam5_AICharacter* AICharacter = Cast<ATeam5_AICharacter>(GetOwner()))
		{
			AICharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AICharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AICharacter->SetLifeSpan(5.f);
		}
	}
}
