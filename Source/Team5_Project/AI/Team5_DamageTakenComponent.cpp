#include "AI/Team5_DamageTakenComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UTeam5_DamageTakenComponent::UTeam5_DamageTakenComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxHP = 100.0f;
	CurrentHP = MaxHP;
	bIsAlive = true;
	SetIsReplicatedByDefault(true);
}

void UTeam5_DamageTakenComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeam5_DamageTakenComponent, bIsAlive);
}

void UTeam5_DamageTakenComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UTeam5_DamageTakenComponent::OnDamageTaken);
	}
}

void UTeam5_DamageTakenComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                AController* InstigatedBy, AActor* DamageCauser)
{
	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);

	UE_LOG(LogTemp, Warning, TEXT("[피격] HP: %.1f"), CurrentHP);

	if (CurrentHP <= 0.0f)
	{
		if (!bIsAlive) return;

		bIsAlive = false;

		APawn* MyPawn = Cast<APawn>(GetOwner());
		if (!MyPawn) return;
		
		DeathCollision(MyPawn);

		if (APlayerController* PlayerController = Cast<APlayerController>(MyPawn->GetController()))
		{
			PlayerController->StopMovement();
			MyPawn->DisableInput(PlayerController);
		}
		else if (AAIController* AIController = Cast<AAIController>(MyPawn->GetController()))
		{
			if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
			{
				BrainComp->StopLogic(TEXT("Dead"));
			}
			AIController->StopMovement();
		}

		if (GetOwner()->HasAuthority() && OnDeathDelegate.IsBound())
		{
			OnDeathDelegate.Broadcast();
		}
	}
}

void UTeam5_DamageTakenComponent::OnRep_IsAlive()
{
	if (!bIsAlive)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAlive: Character Died!"));
		if (OnDeathDelegate.IsBound())
		{
			OnDeathDelegate.Broadcast();
		}
	}
}

void UTeam5_DamageTakenComponent::DeathCollision(APawn* MyPawn)
{
	if (!MyPawn) return;

	if (UCapsuleComponent* CapsuleComp = MyPawn->FindComponentByClass<UCapsuleComponent>())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (ACharacter* Character = Cast<ACharacter>(MyPawn))
	{
		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	else if (USkeletalMeshComponent* MeshComp = MyPawn->FindComponentByClass<USkeletalMeshComponent>())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	MyPawn->SetLifeSpan(5.f);
}
