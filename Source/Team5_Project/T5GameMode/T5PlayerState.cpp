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
    // 역할이 바뀌었을 때 클라이언트에서 처리할 로직 (UI 갱신 등)
}

void AT5PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
    if (HasAuthority())
    {
        CurrentRole = NewRole;
        // 서버에서도 OnRep 로직(UI갱신 등)이 필요하면 명시적 호출
        OnRep_CurrentRole(); 
    }
}

void AT5PlayerState::AddScore(float ScoreToAdd)
{
    // PlayerState의 기본 점수 필드 사용
    SetScore(GetScore() + ScoreToAdd);
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

        // [디버그] 로그 확인용
        // UE_LOG(LogTemp, Warning, TEXT("[HP] %s 체력 감소: %.1f (남은 체력: %.1f)"), *GetPlayerName(), Amount, CurrentHP);
    }
}