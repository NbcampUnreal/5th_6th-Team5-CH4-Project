#include "T5GameMode/T5GameState.h"
#include "Net/UnrealNetwork.h"

AT5GameState::AT5GameState()
{
    CurrentMatchState = EMatchState::Waiting;
    RemainingTime = 60; 
    SurvivorCount = 0;
    
    // [초기화] 아직 승자 없음
    WinningTeam = EWinningTeam::None;
}

void AT5GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AT5GameState, CurrentMatchState);
    DOREPLIFETIME(AT5GameState, RemainingTime);
    DOREPLIFETIME(AT5GameState, SurvivorCount);
    
    // [복제 등록] 이 값이 변하면 모든 클라이언트에게 전송됨
    DOREPLIFETIME(AT5GameState, WinningTeam);
}