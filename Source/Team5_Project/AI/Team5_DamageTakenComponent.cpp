#include "AI/Team5_DamageTakenComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Team5_AICharacter.h"
#include "Components/CapsuleComponent.h"
#include "PlayerCharacter/PlayerCharacter.h"

UTeam5_DamageTakenComponent::UTeam5_DamageTakenComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxHP = 100.0f; 
    CurrentHP = MaxHP;
    bIsAlive = true;
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
       if (bIsAlive == false) return;
       bIsAlive = false;

       APawn* MyPawn = Cast<APawn>(GetOwner());
       if (!MyPawn) return;

       if (APlayerController* PlayerController = Cast<APlayerController>(MyPawn->GetController()))
       {
          PlayerController->StopMovement();
          MyPawn->DisableInput(PlayerController);

          if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MyPawn))
          {
             PlayerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
             PlayerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
             PlayerCharacter->SetLifeSpan(5.f);
             
             if (OnDeathDelegate.IsBound()) OnDeathDelegate.Broadcast();
          }
       }
       
       else if (AAIController* AIController = Cast<AAIController>(MyPawn->GetController()))
       {
           if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
           {
              BrainComp->StopLogic(TEXT("Dead"));
           }
           AIController->StopMovement();

           if (ATeam5_AICharacter* AICharacter = Cast<ATeam5_AICharacter>(MyPawn))
           {
              AICharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
              AICharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
              AICharacter->SetLifeSpan(5.f);
              
              if (OnDeathDelegate.IsBound()) OnDeathDelegate.Broadcast();
           }
       }
    }
}