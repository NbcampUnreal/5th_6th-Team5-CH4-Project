#include "AI/Team5_DamageTakenComponent.h"

#include "PlayerCharacter/T5PlayerController.h"

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

void UTeam5_DamageTakenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeam5_DamageTakenComponent, CurrentHP);
	DOREPLIFETIME(UTeam5_DamageTakenComponent, MaxHP);
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

void UTeam5_DamageTakenComponent::OnDamageTaken(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

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

			// 플레이어라면 Die UI를 해당 클라이언트에게만 띄움
			if (AT5PlayerController* T5PC = Cast<AT5PlayerController>(PlayerController))
			{
				T5PC->Client_ShowDieUI();
			}
		}
		else if (AAIController* AIController = Cast<AAIController>(MyPawn->GetController()))
		{
			if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
			{
				BrainComp->StopLogic(TEXT("Dead"));
			}
			AIController->StopMovement();
		}

		// 서버 기준 죽음 이벤트 브로드캐스트
		if (OnDeathDelegate.IsBound())
		{
			OnDeathDelegate.Broadcast();
		}
	}
}

void UTeam5_DamageTakenComponent::OnRep_CurrentHP()
{
	// 필요하면 여기서 UI 갱신 이벤트를 Broadcast 하는 식으로 확장 가능.
	// (지금은 Percent 바인딩이면 비워도 정상 동작)
}

void UTeam5_DamageTakenComponent::OnRep_IsAlive()
{
	// bIsAlive가 false로 동기화되었을 때 클라이언트에서도 Death 처리
	if (!bIsAlive)
	{
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
