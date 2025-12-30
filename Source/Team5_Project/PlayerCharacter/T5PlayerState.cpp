#include "T5PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/PlayerCharacter.h"

AT5PlayerState::AT5PlayerState()
{
    CurrentRole = EPlayerRole::Waiting;
    CurrentHP = 100.0f;
    bIsDead = false;

    bReplicates = true;
}

void AT5PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AT5PlayerState, CurrentRole);
    DOREPLIFETIME(AT5PlayerState, CurrentHP);
    DOREPLIFETIME(AT5PlayerState, CharacterID);
    DOREPLIFETIME(AT5PlayerState, bIsDead);
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
    // 서버만 HP/죽음 확정
    if (!HasAuthority())
    {
        return;
    }

    // 이미 죽었으면 추가 처리 X (중복 방지)
    if (bIsDead)
    {
        return;
    }

    CurrentHP -= Amount;
    if (CurrentHP <= 0.0f)
    {
        CurrentHP = 0.0f;

        // 죽음 확정
        bIsDead = true;

        // 서버에서도 바로 반응시키고 싶으면 호출(선택)
        OnRep_IsDead();
    }
}

void AT5PlayerState::OnRep_IsDead()
{
    // 여기서는 "죽었다"는 사실만 반영하는 자리.
    // UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Dead replicated!"));
}
