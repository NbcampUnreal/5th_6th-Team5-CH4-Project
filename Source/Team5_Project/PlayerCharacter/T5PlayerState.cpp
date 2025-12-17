#include "T5PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


AT5PlayerState::AT5PlayerState()
{
    // 변수 초기화
    CurrentRole = EPlayerRole::Waiting; 
    CurrentHP = 100.0f;
    bReplicates = true;
}

void AT5PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 변수 동기화 등록
    DOREPLIFETIME(AT5PlayerState, CurrentRole);
    DOREPLIFETIME(AT5PlayerState, CurrentHP);
}

void AT5PlayerState::OnRep_CurrentRole()
{
    
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
    // 서버에서만 로직 수행
    if (GetLocalRole() == ROLE_Authority)
    {
        CurrentHP -= Amount;
        if (CurrentHP < 0.0f)
        {
            CurrentHP = 0.0f;
        }

    }
}