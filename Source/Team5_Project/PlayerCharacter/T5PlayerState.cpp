#include "T5PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


AT5PlayerState::AT5PlayerState()
{
    CurrentRole = EPlayerRole::Waiting; 
    CurrentHP = 100.0f;
    bReplicates = true;
}

void AT5PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AT5PlayerState, CurrentRole);
    DOREPLIFETIME(AT5PlayerState, CurrentHP);
}

void AT5PlayerState::OnRep_CurrentRole()
{
    if (APawn* MyPawn = GetPawn())
    {
        if (APlayerCharacter* MyChar = Cast<APlayerCharacter>(MyPawn))
        {
            MyChar->UpdateCharacterMesh(CurrentRole);
        }
    }
}

void AT5PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
    if (HasAuthority())
    {
        CurrentRole = NewRole;
        OnRep_CurrentRole(); 
    }
}

void AT5PlayerState::ApplyDamage(float Amount)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        CurrentHP -= Amount;
        if (CurrentHP < 0.0f)
        {
            CurrentHP = 0.0f;
        }
    }
}