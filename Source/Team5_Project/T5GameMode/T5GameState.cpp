// T5GameState.cpp

#include "T5GameMode/T5GameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter/PlayerCharacter.h"

AT5GameState::AT5GameState()
{
    // 초기값 설정
    CurrentMatchState = EMatchState::Waiting;
    RemainingTime = 60; // 60초
}

void AT5GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 동기화 등록
    DOREPLIFETIME(AT5GameState, CurrentMatchState);
    DOREPLIFETIME(AT5GameState, RemainingTime);
    DOREPLIFETIME(AT5GameState, SurvivorCount); 
}
